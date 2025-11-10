#include "coppersmith.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

/*
 * Coding this took longer than it needed to :(
 * Coppersmith's method implementation using LLL lattice reduction.
 *
 * The Coppersmith method finds small roots of polynomial equations modulo N.
 * It works by:
 * 1. constructing a lattice of polynomials related to f(x)
 * 2. using LLL to find short vectors in this lattice
 * 3. extracting polynomials with small roots over the integers
 * 4. finding roots using standard polynomial root-finding
 *
 * For the univariate case with polynomial f(x) = ax + b mod N,
 * we want to find small x such that ax + b ≡ 0 (mod p) where p | N.
 */

// matrix type for LLL - no doubles shit, pure bigint
using Matrix = std::vector<std::vector<BigInt> >;

// full fraction type using only bigints shiiiii
struct Fraction {
    BigInt num; // numerator (can be negative)
    BigInt den; // denominator (>0)
    Fraction() : num(BigInt(static_cast<uint64_t>(0))), den(BigInt(static_cast<uint64_t>(1))) {}
    Fraction(const BigInt &n) : num(n), den(BigInt(static_cast<uint64_t>(1))) {}
    Fraction(const BigInt &n, const BigInt &d) : num(n), den(d) { normalize(); }
    void normalize() {
        BigInt zero(static_cast<uint64_t>(0));
        if (den == zero) {
            throw std::runtime_error("fraction denominator is zero");
        }
        // ensure denominator positive
        if (den < zero) {
            den = zero - den;
            num = zero - num;
        }
        // reduce by gcd
        BigInt a = num;
        if (a < zero) a = zero - a; // abs(num)
        BigInt g = BigInt::gcd(a, den);
        if (!(g == BigInt(static_cast<uint64_t>(1)))) {
            num /= g;
            den /= g;
        }
    }
};

// helpers for fraction operations (minimal set)
static Fraction frac_add(const Fraction &a, const Fraction &b) {
    BigInt n = a.num * b.den + b.num * a.den;
    BigInt d = a.den * b.den;
    return Fraction(n, d);
}
static Fraction frac_sub(const Fraction &a, const Fraction &b) {
    BigInt n = a.num * b.den - b.num * a.den;
    BigInt d = a.den * b.den;
    return Fraction(n, d);
}
static Fraction frac_mul(const Fraction &a, const Fraction &b) {
    BigInt n = a.num * b.num;
    BigInt d = a.den * b.den;
    return Fraction(n, d);
}
static Fraction frac_square(const Fraction &a) {
    BigInt n = a.num * a.num;
    BigInt d = a.den * a.den;
    return Fraction(n, d);
}

// compare fractions: returns -1 if a<b, 0 if equal, 1 if a>b
static int frac_cmp(const Fraction &a, const Fraction &b) {
    BigInt left = a.num * b.den;
    BigInt right = b.num * a.den;
    if (left < right) return -1;
    if (left > right) return 1;
    return 0;
}

// round fraction to nearest integer (ties to floor) using bigint only
static BigInt frac_round_to_int(const Fraction &f) {
    BigInt zero(static_cast<uint64_t>(0));
    if (f.den == BigInt(static_cast<uint64_t>(1))) return f.num; // already integer
    // floor
    BigInt q = f.num / f.den;
    BigInt r = f.num % f.den;
    // check |2r| >= |den|
    BigInt two(static_cast<uint64_t>(2));
    BigInt abs2r = r; if (abs2r < zero) abs2r = zero - abs2r; abs2r *= two;
    BigInt absd = f.den; // positive
    if (abs2r > absd) {
        // adjust depending on sign of num
        if (f.num >= zero) {
            q += BigInt(static_cast<uint64_t>(1));
        } else {
            q -= BigInt(static_cast<uint64_t>(1));
        }
    }
    return q;
}

// compute dot product of integer vector and integer vector
static BigInt dot_int(const std::vector<BigInt> &a, const std::vector<BigInt> &b) {
    BigInt acc(static_cast<uint64_t>(0));
    size_t m = std::min(a.size(), b.size());
    for (size_t i=0; i<m; ++i) acc += a[i] * b[i];
    return acc;
}

// compute dot product of integer vector and rational vector
static Fraction dot_mixed(const std::vector<BigInt> &a, const std::vector<Fraction> &b) {
    Fraction acc; // 0
    size_t m = std::min(a.size(), b.size());
    for (size_t i=0; i<m; ++i) {
        Fraction term(a[i] * b[i].num, b[i].den); // a[i]*b[i]
        acc = frac_add(acc, term);
    }
    return acc;
}

// compute squared norm of rational vector
static Fraction norm_sq(const std::vector<Fraction> &v) {
    Fraction acc; // 0
    for (const auto &x : v) {
        acc = frac_add(acc, frac_square(x));
    }
    return acc;
}

/*
 * Full LLL lattice reduction (delta = 3/4) using bigint-only exact rational arithmetic.
 * Implements Gram-Schmidt orthogonalization with fractions (numerator/denominator bigints).
 * This is slower than floating implementations but exact and suitable for RSA-related code.
 *
 * @param basis - matrix of integer basis vectors (each row is a basis vector)
 * @return true if finished (always true unless pathological zero vectors cause issues)
 */
static bool lll_reduce(Matrix &basis) {
    size_t n = basis.size();
    if (n == 0) return true;
    size_t dim = basis[0].size();

    // gram-schmidt storage
    std::vector<std::vector<Fraction>> b_star(n, std::vector<Fraction>(dim));
    std::vector<Fraction> B(n); // squared norms of b_star
    std::vector<std::vector<Fraction>> mu(n, std::vector<Fraction>(n)); // mu[i][j]

    Fraction delta(Fraction(BigInt(static_cast<uint64_t>(3)), BigInt(static_cast<uint64_t>(4))));
    BigInt zero(static_cast<uint64_t>(0));
    BigInt one(static_cast<uint64_t>(1));

    auto recompute_gs = [&](size_t upto){
        for (size_t i=0; i<=upto && i<n; ++i) {
            // start with b_i as fraction vector
            for (size_t k=0; k<dim; ++k) {
                b_star[i][k] = Fraction(basis[i][k]);
            }
            // subtract projections onto previous b_star
            for (size_t j=0; j<i; ++j) {
                // mu[i][j] = <b_i, b*_j> / B[j]
                Fraction numer = dot_mixed(basis[i], b_star[j]);
                // division by B[j]: (numer.num / numer.den) / (B[j].num / B[j].den) = numer.num * B[j].den / (numer.den * B[j].num)
                Fraction mu_ij;
                if (B[j].num == zero) {
                    mu_ij = Fraction(); // treat as zero
                } else {
                    BigInt mu_num = numer.num * B[j].den;
                    BigInt mu_den = numer.den * B[j].num;
                    mu_ij = Fraction(mu_num, mu_den);
                }
                mu[i][j] = mu_ij;
                // b_star[i] -= mu_ij * b_star[j]
                for (size_t k=0; k<dim; ++k) {
                    Fraction prod(b_star[j][k].num * mu_ij.num, b_star[j][k].den * mu_ij.den);
                    b_star[i][k] = frac_sub(b_star[i][k], prod);
                }
            }
            B[i] = norm_sq(b_star[i]);
        }
    };

    recompute_gs(n-1); // initial GS

    size_t k = 1;
    while (k < n) {
        // size reduction: for j = k-1 .. 0
        for (int j = static_cast<int>(k) - 1; j >= 0; --j) {
            // round mu[k][j]
            BigInt r = frac_round_to_int(mu[k][j]);
            // if r != 0, apply b_k -= r * b_j and update mu[k][*]
            if (!(r == zero)) {
                for (size_t col=0; col<dim; ++col) {
                    basis[k][col] -= r * basis[j][col];
                }
            }
        }
        // update GS for vector k (since we modified b_k during size reduction)
        recompute_gs(k);

        // Lovasz condition: (delta - mu[k][k-1]^2) * B[k-1] <= B[k]
        Fraction mu_sq = frac_square(mu[k][k-1]);
        Fraction left = frac_mul(frac_sub(delta, mu_sq), B[k-1]);
        if (frac_cmp(left, B[k]) <= 0) {
            // condition satisfied
            ++k;
            continue;
        }
        // swap b_k and b_{k-1}
        std::swap(basis[k], basis[k-1]);
        // recompute GS for indices k-1 and k onward
        recompute_gs(k);
        if (k > 1) --k;
    }

    return true;
}

/*
 * construct lattice for coppersmith univariate method
 * for polynomial f(x) = ax + b mod N
 *
 * builds a 2x2 lattice with rows representing coefficients of:
 * [ aX, b ]
 * [ 0,  N ]
 * where X is the root bound.
 */
static Matrix build_coppersmith_lattice_linear(
    const BigInt &a,
    const BigInt &b,
    const BigInt &n,
    const BigInt &x_bound
) {
    int dim = 2;
    Matrix lattice(dim, std::vector<BigInt>(dim, BigInt(static_cast<uint64_t>(0))));

    // polynomial g1(x) = f(xX) = a*x*X + b
    // coeff vector: [a*X, b]
    lattice[0][0] = a * x_bound;
    lattice[0][1] = b;

    // polynomial g2(x) = N
    // coeff vector: [0, N]
    lattice[1][0] = BigInt(static_cast<uint64_t>(0));
    lattice[1][1] = n;

    return lattice;
}

// helper: try to extract small root from reduced lattice vector
static std::optional<BigInt> extract_root_from_vector(
    const std::vector<BigInt> &vec,
    const BigInt &x_bound
) {
    // the reduced vector gives a new polynomial with small coefficients
    // vec = [c1, c0] corresponding to polynomial p(x) = c1*x + c0
    // we want root: c1*x + c0 = 0 => x = -c0/c1
    if (vec.size() < 2 || vec[0].is_zero()) {
        return std::nullopt;
    }

    BigInt c1 = vec[0];
    BigInt c0 = vec[1];
    BigInt zero(static_cast<uint64_t>(0));

    // handle negative c0
    BigInt neg_c0 = zero - c0;

    // x = -c0 / c1, check if it divides evenly
    if ((neg_c0 % c1).is_zero()) {
        BigInt root_candidate = neg_c0 / c1;

        // make positive if negative
        if (root_candidate < zero) {
            root_candidate = zero - root_candidate;
        }

        if (root_candidate < x_bound) {
            return root_candidate;
        }
    }

    return std::nullopt;
}

// brute force search for small roots (fallback when LLL doesn't work)
static std::optional<BigInt> brute_force_small_root(
    const BigInt &a,
    const BigInt &b,
    const BigInt &n,
    const BigInt &x_bound
) {
    // check ax + b ≡ 0 (mod n) for x in [0, x_bound]
    BigInt zero(static_cast<uint64_t>(0));
    BigInt one(static_cast<uint64_t>(1));

    // optimization: if we can find mod inverse of a, x = -b/a mod n
    if (auto a_inv = BigInt::mod_inverse(a, n)) {
        BigInt x_candidate = ((*a_inv) * (n - b)) % n;
        // check if it's in the small range
        if (x_candidate < x_bound) {
            BigInt check = (a * x_candidate + b) % n;
            if (check.is_zero()) {
                return x_candidate;
            }
        }
    }

    // fallback brute force for very small bounds
    if (x_bound < BigInt(static_cast<uint64_t>(1000000))) {
        for (BigInt x = zero; x < x_bound; x += one) {
            if (((a * x + b) % n).is_zero()) {
                return x;
            }
        }
    }

    return std::nullopt;
}

CoppersmithResult coppersmith_univariate_linear(
    const BigInt &a,
    const BigInt &b,
    const BigInt &n,
    double beta,
    double epsilon
) {
    CoppersmithResult result;
    std::ostringstream log;

    // based on howgrave-graham, bound is roughly n^beta
    // compute n^beta by taking floor(n^(beta)) as an approximation
    BigInt x_bound(static_cast<uint64_t>(1));
    if (beta > 0) {
        // approximate n^beta using bit_length: n ≈ 2^bit_length
        // so n^beta ≈ 2^(beta * bit_length)
        size_t target_bits = static_cast<size_t>(beta * n.bit_length());
        x_bound = BigInt(static_cast<uint64_t>(1));
        for (size_t i = 0; i < target_bits; ++i) {
            x_bound *= BigInt(static_cast<uint64_t>(2));
        }
    }

    log << "using LLL for root x < " << x_bound.to_dec() << " (N^" << beta << "); ";

    // quick check: direct modular root (if gcd(a, n) == 1)
    if (auto a_inv = BigInt::mod_inverse(a, n)) {
        BigInt root_mod_n = ((*a_inv) * ((n - (b % n)) % n)) % n; // (-b * a^{-1}) mod n
        log << "actual root mod n=" << root_mod_n.to_dec() << "; ";
        if (root_mod_n < x_bound) {
            BigInt check = (a * root_mod_n + b) % n;
            if (check.is_zero()) {
                result.success = true;
                result.root = root_mod_n;
                log << "root is already < bound; returning without lattice";
                result.log = log.str();
                return result;
            }
        } else {
            log << "root not < bound; this is expected failure for chosen beta; ";
        }
    } else {
        log << "a not invertible mod n (gcd != 1); proceeding with lattice; ";
    }

    // try LLL approach first (will usually not help if root >= bound)
    if (x_bound > BigInt(static_cast<uint64_t>(0))) {
        auto lattice = build_coppersmith_lattice_linear(a, b, n, x_bound);

        if (lll_reduce(lattice)) {
            log << "LLL reduction succeeded; ";
            // the first vector in the reduced basis is the shortest
            if (auto root = extract_root_from_vector(lattice[0], x_bound)) {
                BigInt check = (a * (*root) + b) % n;
                if (check.is_zero()) {
                    result.success = true;
                    result.root = *root;
                    log << "found x=" << root->to_dec();
                    result.log = log.str();
                    return result;
                }
            }
            log << "couldn't extract root from lattice; ";
        } else {
            log << "LLL failed to converge; ";
        }
    }

    // fallback to brute force (mostly for very small bounds)
    log << "trying brute force";
    if (auto root = brute_force_small_root(a, b, n, x_bound)) {
        result.success = true;
        result.root = *root;
        log << "; found x=" << root->to_dec();
    } else {
        log << "; no small root found";
    }

    result.log = log.str();
    return result;
}

// helper to evaluate polynomial mod n
static BigInt eval_poly_mod(const std::vector<BigInt> &coeffs, const BigInt &x, const BigInt &n) {
    if (coeffs.empty()) return BigInt(static_cast<uint64_t>(0));
    BigInt result(static_cast<uint64_t>(0));
    for (int i = coeffs.size() - 1; i >= 0; --i) {
        result = (result * x + coeffs[i]) % n;
    }
    return result;
}

// brute force search for polynomial roots (for small search space)
static std::optional<BigInt> brute_force_poly_root(
    const std::vector<BigInt> &coeffs,
    const BigInt &n,
    const BigInt &x_bound
) {
    BigInt zero(static_cast<uint64_t>(0));
    BigInt one(static_cast<uint64_t>(1));

    unsigned long long max_tries = 1000000ULL;
    if (x_bound < BigInt(max_tries)) {
        for (BigInt x = zero; x < x_bound; x += one) {
            if (eval_poly_mod(coeffs, x, n).is_zero()) {
                return x;
            }
        }
    }

    return std::nullopt;
}

CoppersmithResult coppersmith_small_e_partial_msg(
    const BigInt &c,
    unsigned e,
    const BigInt &n,
    const BigInt &m_high,
    size_t unknown_bits
) {
    CoppersmithResult result;
    std::ostringstream log;

    if (e != 3 && e != 5) {
        log << "only e=3 or e=5 supported in this implementation";
        result.log = log.str();
        return result;
    }

    // compute 2^unknown_bits
    BigInt x_bound(static_cast<uint64_t>(1));
    for (size_t i = 0; i < unknown_bits; ++i) {
        x_bound *= BigInt(static_cast<uint64_t>(2));
    }

    log << "searching for " << unknown_bits << " unknown bits (x < " << x_bound.to_dec() << "); ";

    // construct polynomial p(x) = (m_high + x)^e - c ≡ 0 (mod n)
    std::vector<BigInt> coeffs;
    BigInt one(static_cast<uint64_t>(1));
    BigInt m_shifted = m_high;

    if (e == 3) {
        // (x+m)^3 - c
        BigInt m2 = (m_shifted * m_shifted) % n;
        BigInt m3 = (m2 * m_shifted) % n;
        BigInt three(static_cast<uint64_t>(3));
        coeffs = {(m3 - c + n) % n, (three * m2) % n, (three * m_shifted) % n, one};
    } else if (e == 5) {
        // (x+m)^5 - c
        BigInt m2 = (m_shifted * m_shifted) % n;
        BigInt m3 = (m2 * m_shifted) % n;
        BigInt m4 = (m3 * m_shifted) % n;
        BigInt m5 = (m4 * m_shifted) % n;
        BigInt five(static_cast<uint64_t>(5));
        BigInt ten(static_cast<uint64_t>(10));
        coeffs = {
            (m5 - c + n) % n, (five * m4) % n, (ten * m3) % n, (ten * m2) % n,
            (five * m_shifted) % n, one
        };
    }

    // brute force search for small root
    log << "using brute force";
    if (auto root_opt = brute_force_poly_root(coeffs, n, x_bound)) {
        result.success = true;
        result.root = (m_high + *root_opt);
        log << "; recovered missing bits, full message=" << result.root.to_hex();
    } else {
        log << "; failed to find root in search space";
    }

    result.log = log.str();
    return result;
}
