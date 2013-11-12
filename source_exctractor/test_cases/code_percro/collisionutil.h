#ifndef COLLISIONUTIL_H
#define COLLISIONUTIL_H

#define EPSILON 0.0001f


// T(s,t) = B + sE0 + tE1
// http://www.geometrictools.com/LibMathematics/Distance/Wm5DistPoint3Triangle3.cpp
inline Vector3 closestPtPointTriangle(Vector3 P, Vector3 A, Vector3 B, Vector3 C, Scalar& sqrDistance)
{
    // Check if P in vertex region outside A
    Vector3 e0 = B - A; // E0
    Vector3 e1 = C - A; // E1
    Vector3 D = A - P; // D =

    Scalar a00 = e0.squaredNorm();
    Scalar a01 = e0.dot(e1);
    Scalar a11 = e1.squaredNorm();
    Scalar b0 = D.dot(e0);
    Scalar b1 = D.dot(e1);
    Scalar c = D.squaredNorm();
    Scalar det = fabs(a00*a11 - a01*a01);
    Scalar s = a01*b1 - a11*b0;
    Scalar t = a01*b0 - a00*b1;
    sqrDistance = 0;

    if (s + t <= det)
    {
        if (s < (Scalar)0)
        {
            if (t < (Scalar)0)  // region 4
            {
                if (b0 < (Scalar)0)
                {
                    t = (Scalar)0;
                    if (-b0 >= a00)
                    {
                        s = (Scalar)1;
                        sqrDistance = a00 + ((Scalar)2)*b0 + c;
                    }
                    else
                    {
                        s = -b0/a00;
                        sqrDistance = b0*s + c;
                    }
                }
                else
                {
                    s = (Scalar)0;
                    if (b1 >= (Scalar)0)
                    {
                        t = (Scalar)0;
                        sqrDistance = c;
                    }
                    else if (-b1 >= a11)
                    {
                        t = (Scalar)1;
                        sqrDistance = a11 + ((Scalar)2)*b1 + c;
                    }
                    else
                    {
                        t = -b1/a11;
                        sqrDistance = b1*t + c;
                    }
                }
            }
            else  // region 3
            {
                s = (Scalar)0;
                if (b1 >= (Scalar)0)
                {
                    t = (Scalar)0;
                    sqrDistance = c;
                }
                else if (-b1 >= a11)
                {
                    t = (Scalar)1;
                    sqrDistance = a11 + ((Scalar)2)*b1 + c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t + c;
                }
            }
        }
        else if (t < (Scalar)0)  // region 5
        {
            t = (Scalar)0;
            if (b0 >= (Scalar)0)
            {
                s = (Scalar)0;
                sqrDistance = c;
            }
            else if (-b0 >= a00)
            {
                s = (Scalar)1;
                sqrDistance = a00 + ((Scalar)2)*b0 + c;
            }
            else
            {
                s = -b0/a00;
                sqrDistance = b0*s + c;
            }
        }
        else  // region 0
        {
            // minimum at interior point
            Scalar invDet = ((Scalar)1)/det;
            s *= invDet;
            t *= invDet;
            sqrDistance = s*(a00*s + a01*t + ((Scalar)2)*b0) +
                t*(a01*s + a11*t + ((Scalar)2)*b1) + c;
        }
    }
    else
    {
        Scalar tmp0, tmp1, numer, denom;

        if (s < (Scalar)0)  // region 2
        {
            tmp0 = a01 + b0;
            tmp1 = a11 + b1;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00 - ((Scalar)2)*a01 + a11;
                if (numer >= denom)
                {
                    s = (Scalar)1;
                    t = (Scalar)0;
                    sqrDistance = a00 + ((Scalar)2)*b0 + c;
                }
                else
                {
                    s = numer/denom;
                    t = (Scalar)1 - s;
                    sqrDistance = s*(a00*s + a01*t + ((Scalar)2)*b0) +
                        t*(a01*s + a11*t + ((Scalar)2)*b1) + c;
                }
            }
            else
            {
                s = (Scalar)0;
                if (tmp1 <= (Scalar)0)
                {
                    t = (Scalar)1;
                    sqrDistance = a11 + ((Scalar)2)*b1 + c;
                }
                else if (b1 >= (Scalar)0)
                {
                    t = (Scalar)0;
                    sqrDistance = c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t + c;
                }
            }
        }
        else if (t < (Scalar)0)  // region 6
        {
            tmp0 = a01 + b1;
            tmp1 = a00 + b0;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00 - ((Scalar)2)*a01 + a11;
                if (numer >= denom)
                {
                    t = (Scalar)1;
                    s = (Scalar)0;
                    sqrDistance = a11 + ((Scalar)2)*b1 + c;
                }
                else
                {
                    t = numer/denom;
                    s = (Scalar)1 - t;
                    sqrDistance = s*(a00*s + a01*t + ((Scalar)2)*b0) +
                        t*(a01*s + a11*t + ((Scalar)2)*b1) + c;
                }
            }
            else
            {
                t = (Scalar)0;
                if (tmp1 <= (Scalar)0)
                {
                    s = (Scalar)1;
                    sqrDistance = a00 + ((Scalar)2)*b0 + c;
                }
                else if (b0 >= (Scalar)0)
                {
                    s = (Scalar)0;
                    sqrDistance = c;
                }
                else
                {
                    s = -b0/a00;
                    sqrDistance = b0*s + c;
                }
            }
        }
        else  // region 1
        {
            numer = a11 + b1 - a01 - b0;
            if (numer <= (Scalar)0)
            {
                s = (Scalar)0;
                t = (Scalar)1;
                sqrDistance = a11 + ((Scalar)2)*b1 + c;
            }
            else
            {
                denom = a00 - ((Scalar)2)*a01 + a11;
                if (numer >= denom)
                {
                    s = (Scalar)1;
                    t = (Scalar)0;
                    sqrDistance = a00 + ((Scalar)2)*b0 + c;
                }
                else
                {
                    s = numer/denom;
                    t = (Scalar)1 - s;
                    sqrDistance = s*(a00*s + a01*t + ((Scalar)2)*b0) +
                        t*(a01*s + a11*t + ((Scalar)2)*b1) + c;
                }
            }
        }
    }

    // Account for numerical round-off error.
    if (sqrDistance < (Scalar)0)
        sqrDistance = (Scalar)0;
    return A + s*e0 + t*e1;
}


// Intersect segment S(t)=sa+t(sb-sa), 0<=t<=1 against cylinder specified by p, q and r
int intersectSegmentCylinder(Vector3 sa, Vector3 sb, Vector3 p, Vector3 q, Scalar r, Scalar &t)
{
    Vector3 d = q - p , m = sa - p, n = sb - sa;
    Scalar md = m.dot(d);
    Scalar nd = n.dot(d);
    Scalar dd = d.dot(d);
    // Test if segment fully outside either endcap of cylinder
    if (md < 0.0f && md + nd < 0.0f) return 0; // Segment outside ‘p’ side of cylinder
    if (md > dd && md + nd > dd) return 0;     // Segment outside ‘q’ side of cylinder
    Scalar nn = n.dot(n);
    Scalar mn = m.dot(n);
    Scalar a = dd*nn-nd*nd;
    Scalar k = m.dot(m) - r * r;
    Scalar c = dd * k - md * md;
    if (abs(a) < EPSILON) {
        // Segment runs parallel to cylinder axis
        if (c > 0.0f) return 0; // ‘a’ and thus the segment lie outside cylinder
        // Now known that segment intersects cylinder; figure out how it intersects
        if (md < 0.0f) t = -mn / nn; // Intersect segment against ‘p’ endcap
        else if (md > dd) t = (nd - mn) / nn; // Intersect segment against ‘q’ endcap
        else t = 0.0f; // ‘a’ lies inside cylinder
        return 1;
    }
    Scalar b = dd * mn - nd * md;
    Scalar discr = b * b - a * c;
    if (discr < 0.0f) return 0; // No real roots; no intersection
    t = (-b - sqrt(discr)) / a;
    if (t < 0.0f || t > 1.0f) return 0; // Intersection lies outside segment
    if (md + t * nd < 0.0f) {
        // Intersection outside cylinder on ‘p’ side
        if (nd <= 0.0f) return 0; // Segment pointing away from endcap
        t = -md / nd;
        // Keep intersection if Dot(S(t) - p, S(t) - p) <= r^2
        return k + 2 * t * (mn + t * nn) <= 0.0f;
    } else if (md + t * nd > dd) {
        // Intersection outside cylinder on ‘q’ side
        if (nd >= 0.0f) return 0; // Segment pointing away from endcap
        t = (dd - md) / nd;
        // Keep intersection if Dot(S(t) - q, S(t) - q) <= r^2
        return (k + dd - 2 * md + t * (2 * (mn - nd) + t * nn)) <= 0.0f;
    }
    // Segment intersects cylinder between the end-caps; t is correct
    return 1;
}

// project a triangle along the vector DN
inline void projectTriangle(Scalar &min,Scalar &max,Triangle & tri, Vector3 DN)
{
    Scalar p0 = DN.dot(tri.v[0]);
    Scalar p1 = DN.dot(tri.v[1]);
    Scalar p2 = DN.dot(tri.v[2]);

    min = (p1-p0) >=0.0f ?  (p0-p2) >=0.0f ? p2: p0 : (p1-p2) >=0.0f ? p2: p1 ;
    max = (p0-p1) >=0.0f ?  (p2-p0) >=0.0f ? p2: p0 : (p2-p1) >=0.0f ? p2: p1 ;

}


// Clamp n to lie within the range [min, max]
inline Scalar clamp(Scalar n, Scalar min, Scalar max) {
    if (n < min) return min;
    if (n > max) return max;
    return n;
}

// Computes closest points C1 and C2 of S1(s)=P1+s*(Q1-P1) and
// S2(t)=P2+t*(Q2-P2), returning s and t. Function result is squared
// distance between between S1(s) and S2(t)
inline Scalar closestPtSegmentSegment(Vector3 p1, Vector3 q1, Vector3 p2, Vector3 q2,
                              Scalar &s, Scalar &t, Vector3 &c1, Vector3 &c2)
{
    Vector3 d1 = q1 - p1; // Direction vector of segment S1
    Vector3 d2 = q2 - p2; // Direction vector of segment S2
    Vector3 r = p1 - p2;
    Scalar a = d1.dot(d1); // Squared length of segment S1, always nonnegative
    Scalar e = d2.dot(d2); // Squared length of segment S2, always nonnegative
    Scalar f = d2.dot(r);

    // Check if either or both segments degenerate into points
    if (a <= EPSILON && e <= EPSILON) {
        // Both segments degenerate into points
        s = t = 0.0f;
        c1 = p1;
        c2 = p2;
        return (c1 - c2).dot(c1 - c2);
    }
    if (a <= EPSILON) {
        // First segment degenerates into a point
        s = 0.0f;
        t = f / e; // s = 0 => t = (b*s + f) / e = f / e
        t = clamp(t, 0.0f, 1.0f);
    } else {
        Scalar c = d1.dot(r);
        if (e <= EPSILON) {
            // Second segment degenerates into a point
            t = 0.0f;
            s = clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
        } else {
            // The general nondegenerate case starts here
            Scalar b = d1.dot(d2);
            Scalar denom = a*e-b*b; // Always nonnegative

            // If segments not parallel, compute closest point on L1 to L2, and
            // clamp to segment S1. Else pick arbitrary s (here 0)
            if (denom != 0.0f) {
                s = clamp((b*f - c*e) / denom, 0.0f, 1.0f);
            } else s = 0.0f;

            // Compute point on L2 closest to S1(s) using
            // t = Dot((P1+D1*s)-P2,D2) / Dot(D2,D2) = (b*s + f) / e
            // If t in [0,1] done. Else clamp t, recompute s for the new value
            // of t using s = Dot((P2+D2*t)-P1,D1) / Dot(D1,D1)= (t*b - c) / a
            // and clamp s to [0, 1]

            Scalar tnom = b*s + f;
            if (tnom < 0.0f) {
                t = 0.0f;
                s = clamp(-c / a, 0.0f, 1.0f);
            } else if (tnom > e) {
                t = 1.0f;
                s = clamp((b - c) / a, 0.0f, 1.0f);
            } else {
                t = tnom / e;
            }
        }
    }

    c1 = p1 + d1 * s;
    c2 = p2 + d2 * t;
    return (c1 - c2).dot(c1 - c2);
}



#endif // COLLISIONUTIL_H

