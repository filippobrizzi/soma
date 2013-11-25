void CodeGenFunction::EmitOMPTaskDirective(const OMPTaskDirective &S) {
  // Generate shared args for captured stmt.
  CapturedStmt *CS = cast<CapturedStmt>(S.getAssociatedStmt());
  llvm::Value *Arg = GenerateCapturedStmtArgument(*CS);

  // Init list of private globals in the stack.
  CGM.OpenMPSupport.startOpenMPRegion(true);
  CGM.OpenMPSupport.setMergeable(false);
  CGM.OpenMPSupport.setOrdered(false);
  CGM.OpenMPSupport.setUntied(false);
  CGM.OpenMPSupport.setScheduleChunkSize(KMP_SCH_DEFAULT, 0);

  RecordDecl *RD;
  RD = RecordDecl::Create(getContext(), TTK_Struct,
                            getContext().getTranslationUnitDecl(),
                            SourceLocation(), SourceLocation(),
                            &getContext().Idents.get(".omp.task.priv."));
  
  RD->startDefinition();
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I) {
    if (OMPPrivateClause *C = dyn_cast_or_null<OMPPrivateClause>(*I)) {
      for (OMPPrivateClause::varlist_const_iterator II = C->varlist_begin(),
                                                    EE = C->varlist_end();
           II != EE; ++II) {
        const ValueDecl *D = cast<DeclRefExpr>(*II)->getDecl();
        FieldDecl *FD = FieldDecl::Create(getContext(), RD, SourceLocation(),
                                          SourceLocation(), D->getIdentifier(),
                                          (*II)->getType(), 0, 0, false,
                                          ICIS_NoInit);
        FD->setAccess(AS_public);
        RD->addDecl(FD);
        CGM.OpenMPSupport.getTaskFields()[D] = FD;
      }
    } else if (OMPFirstPrivateClause *C =
                dyn_cast_or_null<OMPFirstPrivateClause>(*I)) {
      for (OMPFirstPrivateClause::varlist_const_iterator II = C->varlist_begin(),
                                                         EE = C->varlist_end();
           II != EE; ++II) {
        const ValueDecl *D = cast<DeclRefExpr>(*II)->getDecl();
        FieldDecl *FD = FieldDecl::Create(getContext(), RD, SourceLocation(),
                                          SourceLocation(), D->getIdentifier(),
                                          (*II)->getType(), 0, 0, false,
                                          ICIS_NoInit);
        FD->setAccess(AS_public);
        RD->addDecl(FD);
        CGM.OpenMPSupport.getTaskFields()[D] = FD;
      }
    }
  }
  RD->completeDefinition();
  QualType PrivateRecord = getContext().getRecordType(RD);
  llvm::Type *LPrivateTy = getTypes().ConvertTypeForMem(PrivateRecord);

// llvm::Type *PTaskFnTy = llvm::TypeBuilder<kmp_routine_entry_t, false>::get(getLLVMContext());
// llvm::AllocaInst *FnPtr = CreateTempAlloca(PTaskFnTy);
// FnPtr->setAlignment(llvm::ConstantExpr::getAlignOf(PTaskFnTy));

  // CodeGen for clauses (task init).
  llvm::AllocaInst *Flags = CreateMemTemp(getContext().IntTy, ".flags.addr");
  CGM.OpenMPSupport.setTaskFlags(Flags);

  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I) EmitInitOMPClause(*(*I), S);

  InitTempAlloca(Flags, Builder.getInt32(CGM.OpenMPSupport.getUntied() ?
                                             OMP_TASK_UNTIED : OMP_TASK_TIED));

  // Generate microtask.
  // int32 .omp_ptask.(int32_t arg1, void */*kmp_task_t **/arg2) {
  // captured_stmt(arg2->shareds);
  // }
  IdentifierInfo *Id = &getContext().Idents.get(".omp_ptask.");
  SmallVector<QualType, 2> FnArgTypes;
  FnArgTypes.push_back(getContext().IntTy);
  FnArgTypes.push_back(getContext().VoidPtrTy);
  FunctionProtoType::ExtProtoInfo EPI;
  EPI.ExceptionSpecType = EST_BasicNoexcept;
  QualType FnTy =
    getContext().getFunctionType(getContext().IntTy, FnArgTypes,
                                 EPI);
  TypeSourceInfo *TI =
         getContext().getTrivialTypeSourceInfo(FnTy,
                                               SourceLocation());
  FunctionDecl *FD = FunctionDecl::Create(getContext(),
                                          getContext().getTranslationUnitDecl(),
                                          CS->getLocStart(), SourceLocation(),
                                          Id, FnTy, TI, SC_Static, false, false,
                                          false);
  TypeSourceInfo *IntTI =
         getContext().getTrivialTypeSourceInfo(getContext().IntTy,
                                               SourceLocation());
  TypeSourceInfo *PtrVoidTI =
         getContext().getTrivialTypeSourceInfo(getContext().VoidPtrTy,
                                               SourceLocation());
  ParmVarDecl *Arg1 = ParmVarDecl::Create(getContext(), FD, SourceLocation(),
                                          SourceLocation(), 0,
                                          getContext().IntTy, IntTI,
                                          SC_Auto, 0);
  ParmVarDecl *Arg2 = ParmVarDecl::Create(getContext(), FD, SourceLocation(),
                                          SourceLocation(), 0,
                                          getContext().VoidPtrTy, PtrVoidTI,
                                          SC_Auto, 0);
  CodeGenFunction CGF(CGM, true);
  const CGFunctionInfo &FI = getTypes().arrangeFunctionDeclaration(FD);
  llvm::Function *Fn = llvm::Function::Create(getTypes().GetFunctionType(FI),
                                              llvm::GlobalValue::PrivateLinkage,
                                              FD->getName(), &CGM.getModule());
  CGM.SetInternalFunctionAttributes(CurFuncDecl, Fn, FI);
  llvm::AttributeSet Set = CurFn->getAttributes();
  for (unsigned i = 0; i < Set.getNumSlots(); ++i) {
    if (Set.getSlotIndex(i) == llvm::AttributeSet::FunctionIndex) {
      for (llvm::AttributeSet::iterator I = Set.begin(i), E = Set.end(i); I != E; ++I) {
        if (I->isStringAttribute() && I->getKindAsString().startswith("INTEL:"))
          Fn->addFnAttr(I->getKindAsString());
      }
    }
  }
  FunctionArgList FnArgs;
  FnArgs.push_back(Arg1);
  FnArgs.push_back(Arg2);
  CGF.maybeInitializeDebugInfo();
  CGF.StartFunction(FD, getContext().IntTy, Fn, FI, FnArgs, SourceLocation());
  llvm::AllocaInst *GTid = CGF.CreateMemTemp(getContext().IntTy,
                                             ".__kmpc_global_thread_num.");
  CGF.EmitStoreOfScalar(CGF.Builder.CreateLoad(CGF.GetAddrOfLocalVar(Arg1)),
                        MakeNaturalAlignAddrLValue(GTid, getContext().IntTy),
                        false);
  llvm::Type *TaskTTy = llvm::TaskTBuilder::get(getLLVMContext());
  llvm::Value *TaskTPtr = CGF.Builder.CreatePointerCast(CGF.GetAddrOfLocalVar(Arg2),
                                                        TaskTTy->getPointerTo()->getPointerTo());

  // Emit call to the helper function.
  llvm::Value *Addr = CGF.Builder.CreateConstInBoundsGEP2_32(
                                    CGF.Builder.CreateLoad(TaskTPtr, ".arg2.shareds"), 0,
                                    llvm::TaskTBuilder::shareds, ".arg2.shareds.addr");
  llvm::Value *Arg2Val = CGF.Builder.CreateLoad(Addr, ".arg2.shareds.");
  QualType QTy = getContext().getRecordType(CS->getCapturedRecordDecl());
  llvm::Type *ConvertedType =
                    CGF.getTypes().ConvertTypeForMem(QTy)->getPointerTo();
  llvm::Value *RecArg = CGF.Builder.CreatePointerCast(Arg2Val, ConvertedType, "(anon)shared");

  llvm::Value *Locker = CGF.Builder.CreateConstInBoundsGEP1_32(CGF.Builder.CreateLoad(TaskTPtr), 1);
  CGM.OpenMPSupport.setPTask(Fn, Arg2Val, LPrivateTy, PrivateRecord, Locker);
  // CodeGen for clauses (call start).
  CGF.CapturedStmtInfo = new CGOpenMPCapturedStmtInfo(RecArg, *CS, CGM);
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I) CGF.EmitPreOMPClause(*(*I), S);

  llvm::BasicBlock *UntiedEnd = 0;
  if (CGM.OpenMPSupport.getUntied()) {
    llvm::Value *Addr = CGF.Builder.CreateConstInBoundsGEP2_32(
                                  CGF.Builder.CreateLoad(TaskTPtr, ".arg2.part_id."), 0,
                                  llvm::TaskTBuilder::part_id, ".part_id.addr");
    llvm::Value *PartId = CGF.Builder.CreateLoad(Addr, ".part_id.");
    UntiedEnd = CGF.createBasicBlock("untied.sw.end");
    llvm::SwitchInst *UntiedSwitch = CGF.Builder.CreateSwitch(PartId, UntiedEnd);
    llvm::BasicBlock *InitBlock = CGF.createBasicBlock("untied.sw.init");
    CGF.EmitBlock(InitBlock);
    UntiedSwitch->addCase(CGF.Builder.getInt32(0), InitBlock);
    CGM.OpenMPSupport.setUntiedData(Addr, UntiedSwitch, UntiedEnd, 0);
  }
  CGF.EmitCapturedStmtInlined(*CS, CR_Default, RecArg);
  CGF.EnsureInsertPoint();
  if (UntiedEnd)
    CGF.EmitBlock(UntiedEnd);

  // CodeGen for clauses (call end).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I) CGF.EmitPostOMPClause(*(*I), S);

  // CodeGen for clauses (closing steps).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I) CGF.EmitCloseOMPClause(*(*I), S);

  delete CGF.CapturedStmtInfo;

  CGF.FinishFunction();

  llvm::DenseMap<const ValueDecl *, FieldDecl *> SavedFields = CGM.OpenMPSupport.getTaskFields();
  CGM.OpenMPSupport.endOpenMPRegion();

  // CodeGen for "omp task {Associated statement}".
  CGM.OpenMPSupport.startOpenMPRegion(false);
  CGM.OpenMPSupport.getTaskFields() = SavedFields;
  {
    RunCleanupsScope MainBlock(*this);

    EmitParentUntiedPartIdInc(*this);

    llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
    llvm::Value *GTid =
       CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
    llvm::Value *RealArgs[] = {Loc,
                               GTid,
                               Builder.CreateLoad(Flags, ".flags."),
                               Builder.CreateAdd(
                                        Builder.CreateIntCast(llvm::ConstantExpr::getSizeOf(TaskTTy),
                                                              SizeTy, false),
                                        llvm::ConstantInt::get(SizeTy, getContext().getTypeSizeInChars(PrivateRecord).getQuantity())),
                               llvm::ConstantInt::get(SizeTy, getContext().getTypeSizeInChars(QTy).getQuantity()),
                               Fn};
    // kmpc_task_t val = __kmpc_omp_task_alloc(&loc, gtid, flags, sizeof(kmpc_task_t), sizeof(shareds), task_entry);
    llvm::Value *TaskTVal = EmitRuntimeCall(OPENMPRTL_FUNC(omp_task_alloc), makeArrayRef(RealArgs), ".task_t.val.addr");
    llvm::Value *SharedAddr = Builder.CreateConstInBoundsGEP2_32(
                                        TaskTVal, 0,
                                        llvm::TaskTBuilder::shareds, ".shared.addr");
    EmitAggregateAssign(Builder.CreateLoad(SharedAddr), Arg, QTy);
    llvm::Value *Locker = Builder.CreateConstInBoundsGEP1_32(TaskTVal, 1);
    CGM.OpenMPSupport.setPTask(Fn, TaskTVal, LPrivateTy, PrivateRecord, Locker);
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I && (isa<OMPPrivateClause>(*I) || isa<OMPFirstPrivateClause>(*I)))
        EmitPreOMPClause(*(*I), S);

    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I) EmitAfterInitOMPClause(*(*I), S);

    llvm::Value *RealArgs1[] = {Loc,
                                GTid,
                                TaskTVal};
    if (CGM.OpenMPSupport.getParentUntied()) {
      llvm::Value *Res = EmitRuntimeCall(OPENMPRTL_FUNC(omp_task_parts), makeArrayRef(RealArgs1), ".task.res.");
      llvm::Value *Cond =
            Builder.CreateICmpEQ(Res, Builder.getInt32(OMP_TASK_CURRENT_QUEUED));
      llvm::BasicBlock *ThenBB = createBasicBlock("task.parts.then");
      llvm::BasicBlock *EndBB = createBasicBlock("task.parts.end");
      Builder.CreateCondBr(Cond, ThenBB, EndBB);
      EmitBlock(ThenBB);
      EmitParentUntiedBranchEnd(*this);
      EmitBlock(EndBB, true);
    }
    else
      EmitRuntimeCall(OPENMPRTL_FUNC(omp_task), makeArrayRef(RealArgs1), ".task.res.");
    EmitParentUntiedTaskSwitch(*this, true);
  }

  // CodeGen for clauses (task finalize).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I) EmitFinalOMPClause(*(*I), S);

  // Remove list of private globals from the stack.
  CGM.OpenMPSupport.endOpenMPRegion();
}

