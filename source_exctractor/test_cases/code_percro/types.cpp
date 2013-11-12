#include "types.hpp"




// build from the CSR structure into a ELL with N elements
void from(const SparseIdx & sp)
{
    resize(sp.cols());
    for(unsigned i = 0; i < sp.cols(); i++)
    {
        unsigned  b = sp.outer[i];
        unsigned  e = sp.outer[i+1];
        unsigned m = e-b;
        if (m > N)
            m = N;
        std::fill(std::copy(sp.inner.begin() + b,sp.inner.begin() + (b+m),inner.begin() + i*N),inner.begin() + (i+1)*N,-1);
        innercount[i] = m;
    }
}

// build from the CSR structure into a ELL with N elements, skipping identities (that is i == inner[j] for some j
void fromnoid(const SparseIdx & sp)
{
    resize(sp.cols());
    for(unsigned i = 0; i < sp.cols(); i++)
    {
        unsigned b = sp.outer[i];
        unsigned e = sp.outer[i+1];
        unsigned m = e-b;
        if (m > N+1)
            m = N;
        std::vector<unsigned>::iterator itb = inner.begin() + i*N;
        std::vector<unsigned>::iterator ite = std::remove_copy_if(
                sp.inner.begin() + b,sp.inner.begin() + (b+m),itb,
                    std::bind2nd(std::not_equal_to<unsigned>(), i));
        std::fill(ite,inner.begin() + (i+1)*N,-1);
        innercount[i] = ite-itb;
    }
}

