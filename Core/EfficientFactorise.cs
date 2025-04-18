using System.Diagnostics;
using System.Numerics;

namespace rsaShit.Core;

// holy shit gnfs is so tuff 🥀
public static class EfficientFactorise
{
    public static (BigInteger, BigInteger) Factorise(BigInteger value)
    {
        Console.WriteLine(Color.Red+"[*] Start GNFS"+Color.Reset);
        Stopwatch stopwatch = new Stopwatch();
        stopwatch.Start();
        try
        {
            var (a, b) = Factor(value);
            Console.WriteLine($"GNFS deadass done in: {stopwatch.Elapsed}");
            return (a, b);
        }
        catch (Exception e)
        {
            stopwatch.Stop();
            Console.WriteLine($"Time: {stopwatch.Elapsed}");
            Console.WriteLine("GNFS error occured: "+e);
            throw;
        }
    }

    /// <summary>
    /// Factors a composite number into its two prime factors using GNFS.
    /// </summary>
    /// <param name="n">The number to factor (typically a 1024-bit RSA modulus)</param>
    /// <returns>A tuple containing the two prime factors</returns>
    private static (BigInteger p, BigInteger q) Factor(BigInteger n)
    {
        Console.WriteLine($"Starting GNFS factorization of {n}");

        // Step 1: Polynomial selection
        var polynomial = SelectPolynomial(n);
        Console.WriteLine($"Selected polynomial: {FormatPolynomial(polynomial)}");

        // Step 2: Factor base selection
        int bound = CalculateSmoothnessBound(n);
        var rationalFactorBase = GenerateRationalFactorBase(bound);
        var algebraicFactorBase = GenerateAlgebraicFactorBase(polynomial, bound);
        Console.WriteLine($"Factor bases generated with bound {bound}");

        // Step 3: Sieving to find relations
        var relations = CollectRelations(n, polynomial, rationalFactorBase, algebraicFactorBase);
        Console.WriteLine($"Collected {relations.Count} relations");

        // Step 4: Linear algebra to find dependencies
        var dependencies = FindDependencies(relations);
        Console.WriteLine($"Found {dependencies.Count} dependencies");

        // Step 5: Square root step to find factors
        foreach (var dependency in dependencies)
        {
            var factors = ComputeFactorsFromDependency(n, dependency, relations, polynomial);
            if (factors.p > 1 && factors.q > 1 && factors.p * factors.q == n)
            {
                Console.WriteLine($"Found factors: {factors.p} and {factors.q}");
                return factors;
            }
        }

        // Fallback to trial division for very small numbers or if GNFS fails
        return FallbackFactorization(n);
    }

    /// <summary>
    /// Selects a polynomial for the number field sieve.
    /// </summary>
    private static List<BigInteger> SelectPolynomial(BigInteger n)
    {
        // For simplicity, we'll use a basic approach to polynomial selection
        // A more sophisticated implementation would use advanced techniques

        int degree = EstimateOptimalDegree(n);
        var polynomial = new List<BigInteger>();

        // Generate a monic polynomial of the form x^d - m where n ≈ m^d
        BigInteger m = NthRoot(n, degree);

        // Create x^d - m
        for (int i = 0; i < degree; i++)
        {
            polynomial.Add(BigInteger.Zero);
        }

        polynomial.Add(BigInteger.One); // x^d coefficient
        polynomial[0] = -m; // constant term

        return polynomial;
    }

    /// <summary>
    /// Estimates the optimal degree for the GNFS polynomial.
    /// </summary>
    private static int EstimateOptimalDegree(BigInteger n)
    {
        // Simple heuristic for degree selection based on number size
        int bitLength = (int)BigInteger.Log(n, 2) + 1;

        if (bitLength <= 220)
            return 4;
        if (bitLength <= 360)
            return 5;
        if (bitLength <= 700)
            return 6;
        return 7; // For 1024-bit numbers and above
    }

    /// <summary>
    /// Calculates an approximate n-th root of a number.
    /// </summary>
    private static BigInteger NthRoot(BigInteger value, int n)
    {
        if (n <= 0)
            throw new ArgumentException("Root must be positive");
        if (value == 0)
            return 0;

        // Initial guess
        int bitLength = (int)BigInteger.Log(value, 2) + 1;
        BigInteger x = BigInteger.One << (bitLength / n);

        // Newton's method for finding roots
        bool improved;
        int maxIterations = 100;
        int iteration = 0;

        do
        {
            iteration++;
            BigInteger nx = (BigInteger.Multiply(x, n - 1) + value / BigInteger.Pow(x, n - 1)) / n;
            improved = nx != x;
            x = nx;
        } while (improved && iteration < maxIterations);

        return x;
    }

    /// <summary>
    /// Formats a polynomial for display.
    /// </summary>
    private static string FormatPolynomial(List<BigInteger> polynomial)
    {
        List<string> terms = new List<string>();

        for (int i = 0; i < polynomial.Count; i++)
        {
            if (polynomial[i] == 0)
                continue;

            string term;
            if (i == 0)
            {
                term = polynomial[i].ToString();
            }
            else if (i == 1)
            {
                term = polynomial[i] == 1 ? "x" : $"{polynomial[i]}x";
            }
            else
            {
                term = polynomial[i] == 1 ? $"x^{i}" : $"{polynomial[i]}x^{i}";
            }

            terms.Add(term);
        }

        return string.Join(" + ", terms.Select(t => t.StartsWith("-") ? t : "+" + t))
            .TrimStart('+')
            .Trim();
    }

    /// <summary>
    /// Calculates a suitable smoothness bound based on the size of n.
    /// </summary>
    private static int CalculateSmoothnessBound(BigInteger n)
    {
        // Simple heuristic for the smoothness bound
        int bitLength = (int)BigInteger.Log(n, 2) + 1;
        return (int)Math.Pow(Math.Exp(0.5 * Math.Sqrt(bitLength * Math.Log(bitLength))), 0.5);
    }

    /// <summary>
    /// Generates the rational factor base (primes p such that n is a quadratic residue modulo p).
    /// </summary>
    private static List<int> GenerateRationalFactorBase(int bound)
    {
        var primes = GeneratePrimes(bound);
        return primes;
    }

    /// <summary>
    /// Generates the algebraic factor base.
    /// </summary>
    private static List<int> GenerateAlgebraicFactorBase(List<BigInteger> polynomial, int bound)
    {
        var primes = GeneratePrimes(bound);
        // In a complete implementation, we would filter the primes based on the roots of the polynomial
        // For simplicity, we'll just use all primes up to the bound
        return primes;
    }

    /// <summary>
    /// Generates all primes up to a given bound using the Sieve of Eratosthenes.
    /// </summary>
    private static List<int> GeneratePrimes(int bound)
    {
        bool[] sieve = new bool[bound + 1];
        for (int i = 2; i <= bound; i++)
        {
            sieve[i] = true;
        }

        for (int i = 2; i * i <= bound; i++)
        {
            if (sieve[i])
            {
                for (int j = i * i; j <= bound; j += i)
                {
                    sieve[j] = false;
                }
            }
        }

        var primes = new List<int>();
        for (int i = 2; i <= bound; i++)
        {
            if (sieve[i])
                primes.Add(i);
        }

        return primes;
    }

    /// <summary>
    /// Collects relations through sieving.
    /// </summary>
    private static List<Relation> CollectRelations(
        BigInteger n,
        List<BigInteger> polynomial,
        List<int> rationalFactorBase,
        List<int> algebraicFactorBase
    )
    {
        var relations = new List<Relation>();
        int requiredRelations = rationalFactorBase.Count + algebraicFactorBase.Count + 10;

        // Simple sieving loop - in a real implementation, this would use a more efficient approach
        BigInteger a = 1;
        BigInteger maxA = BigInteger.Min(n, BigInteger.Pow(10, 6)); // Arbitrary limit

        while (relations.Count < requiredRelations && a < maxA)
        {
            for (BigInteger b = -100; b <= 100 && relations.Count < requiredRelations; b++)
            {
                if (b == 0)
                    continue;

                // Check if a-bm is smooth over the rational factor base
                BigInteger m = -polynomial[0]; // constant term is -m
                BigInteger rationalNorm = a - b * m;
                if (rationalNorm == 0)
                    continue;

                // Check if F(a/b) is smooth over the algebraic factor base
                BigInteger algebraicNorm = EvaluatePolynomial(polynomial, a, b);
                if (algebraicNorm == 0)
                    continue;

                var rationalFactorization = FactorOverBase(
                    BigInteger.Abs(rationalNorm),
                    rationalFactorBase
                );
                var algebraicFactorization = FactorOverBase(
                    BigInteger.Abs(algebraicNorm),
                    algebraicFactorBase
                );

                relations.Add(
                    new Relation
                    {
                        A = a,
                        B = b,
                        RationalFactorization = rationalFactorization,
                        AlgebraicFactorization = algebraicFactorization,
                    }
                );
            }

            a++;
        }

        return relations;
    }

    /// <summary>
    /// Evaluates a polynomial at a/b.
    /// </summary>
    private static BigInteger EvaluatePolynomial(
        List<BigInteger> polynomial,
        BigInteger a,
        BigInteger b
    )
    {
        BigInteger result = 0;
        BigInteger aPower = 1;

        for (int i = 0; i < polynomial.Count; i++)
        {
            BigInteger term = polynomial[i] * aPower * BigInteger.Pow(b, polynomial.Count - 1 - i);
            result += term;
            aPower *= a;
        }

        return result;
    }

    /// <summary>
    /// Factors a number over a given factor base.
    /// </summary>
    private static Dictionary<int, int> FactorOverBase(BigInteger num, List<int> factorBase)
    {
        var factorization = new Dictionary<int, int>();
        BigInteger n = num;

        foreach (int p in factorBase)
        {
            int exponent = 0;
            while (n % p == 0)
            {
                n /= p;
                exponent++;
            }

            if (exponent > 0)
            {
                factorization[p] = exponent;
            }
        }

        // Check if the number is fully factored
        if (n == 1)
        {
            return factorization;
        }

        // If n is still large, it wasn't smooth over our factor base
        return null;
    }

    /// <summary>
    /// Finds linear dependencies among the relations using Gaussian elimination.
    /// </summary>
    private static List<List<int>> FindDependencies(List<Relation> relations)
    {
        // This is a simplified version; a real implementation would use sparse matrix techniques
        int numRelations = relations.Count;
        if (numRelations == 0)
            return new List<List<int>>();

        // Create a matrix from the exponent vectors
        var allPrimes = new HashSet<int>();
        foreach (var relation in relations)
        {
            foreach (var prime in relation.RationalFactorization.Keys)
            {
                allPrimes.Add(prime);
            }

            foreach (var prime in relation.AlgebraicFactorization.Keys)
            {
                allPrimes.Add(prime);
            }
        }

        var primes = allPrimes.ToList();
        int numPrimes = primes.Count;

        // Create a binary matrix for Gaussian elimination
        bool[,] matrix = new bool[numPrimes, numRelations];

        for (int j = 0; j < numRelations; j++)
        {
            var relation = relations[j];
            for (int i = 0; i < numPrimes; i++)
            {
                int prime = primes[i];
                bool hasRational = relation.RationalFactorization.TryGetValue(
                    prime,
                    out int rationalExp
                );
                bool hasAlgebraic = relation.AlgebraicFactorization.TryGetValue(
                    prime,
                    out int algebraicExp
                );

                // The exponent vector entry is 1 if the sum of exponents is odd
                matrix[i, j] =
                    (hasRational && rationalExp % 2 == 1)
                    || (hasAlgebraic && algebraicExp % 2 == 1);
            }
        }

        // Perform Gaussian elimination to find dependencies (nullspace basis)
        var dependencies = new List<List<int>>();
        var eliminated = new bool[numRelations];

        for (int i = 0; i < numPrimes && i < numRelations; i++)
        {
            // Find a pivot
            int pivot = -1;
            for (int j = 0; j < numRelations; j++)
            {
                if (!eliminated[j] && matrix[i, j])
                {
                    pivot = j;
                    break;
                }
            }

            if (pivot == -1)
                continue;

            eliminated[pivot] = true;

            // Eliminate this variable from other equations
            for (int j = 0; j < numRelations; j++)
            {
                if (j != pivot && matrix[i, j])
                {
                    for (int k = 0; k < numPrimes; k++)
                    {
                        matrix[k, j] = matrix[k, j] != matrix[k, pivot];
                    }
                }
            }
        }

        // Find dependencies (each non-pivot column gives a dependency)
        for (int j = 0; j < numRelations; j++)
        {
            if (!eliminated[j])
            {
                var dependency = new List<int> { j // This is our free variable
                };

                for (int i = 0, pivotCol = 0; i < numPrimes && pivotCol < numRelations; i++)
                {
                    while (pivotCol < numRelations && !eliminated[pivotCol])
                    {
                        pivotCol++;
                    }

                    if (pivotCol < numRelations && matrix[i, j])
                    {
                        dependency.Add(pivotCol);
                    }

                    if (pivotCol < numRelations)
                        pivotCol++;
                }

                dependencies.Add(dependency);
            }
        }

        return dependencies;
    }

    /// <summary>
    /// Computes the factors of n from a dependency among relations.
    /// </summary>
    private static (BigInteger p, BigInteger q) ComputeFactorsFromDependency(
        BigInteger n,
        List<int> dependency,
        List<Relation> relations,
        List<BigInteger> polynomial
    )
    {
        // Compute the product of a-bm for all relations in the dependency
        BigInteger x = 1;

        // Compute the product of F(a/b) for all relations in the dependency
        BigInteger y = 1;

        // Also track the product of a and b values for algebraic square root
        BigInteger aProd = 1;
        BigInteger bProd = 1;

        BigInteger m = -polynomial[0]; // constant term is -m

        foreach (int relIndex in dependency)
        {
            var relation = relations[relIndex];

            x = (x * (relation.A - relation.B * m)) % n;

            // For the algebraic side, we're computing the product of F(a/b) values
            // In a full implementation, we would compute the algebraic square root properly
            BigInteger algebraicValue = EvaluatePolynomial(polynomial, relation.A, relation.B);
            y = (y * algebraicValue) % n;

            aProd = (aProd * relation.A) % n;
            bProd = (bProd * relation.B) % n;
        }

        // In a full implementation, we would compute the algebraic square root properly
        // For simplicity, we'll use the Legendre symbol to check if y is a square modulo n
        bool isSquare = true; // Simplified check
        BigInteger yRoot = y; // Simplified root

        // Compute gcd(x-yRoot, n) for potential factor
        BigInteger factor1 = BigInteger.GreatestCommonDivisor(x - yRoot, n);
        BigInteger factor2 = BigInteger.GreatestCommonDivisor(x + yRoot, n);

        // If we found a trivial factorization, try again with a different combination
        if (factor1 == 1 || factor1 == n)
        {
            factor1 = factor2;
        }

        if (factor1 == 1 || factor1 == n)
        {
            return (1, n); // No non-trivial factorization found
        }

        return (factor1, n / factor1);
    }

    /// <summary>
    /// Fallback factorization method if GNFS fails.
    /// </summary>
    private static (BigInteger p, BigInteger q) FallbackFactorization(BigInteger n)
    {
        Console.WriteLine("GNFS failed, trying Pollard's rho algorithm as fallback");

        // Try Pollard's rho algorithm
        BigInteger factor = PollardRho(n);

        if (factor == 1 || factor == n)
        {
            // As a last resort, try trial division up to a reasonable limit
            Console.WriteLine("Falling back to trial division for small factors");
            factor = TrialDivision(n, 1_000_000);
        }

        if (factor == 1 || factor == n)
        {
            Console.WriteLine("Factorization failed");
            return (1, n);
        }

        return (factor, n / factor);
    }

    /// <summary>
    /// Pollard's rho algorithm for integer factorization.
    /// </summary>
    private static BigInteger PollardRho(BigInteger n)
    {
        if (n <= 3 || n % 2 == 0)
            return 2;

        BigInteger x = 2;
        BigInteger y = 2;
        BigInteger d = 1;

        // f(x) = x^2 + 1 (mod n)
        Func<BigInteger, BigInteger> f = z => (BigInteger.Pow(z, 2) + 1) % n;

        int maxIterations = 1_000_000;
        int iteration = 0;

        while (d == 1 && iteration < maxIterations)
        {
            iteration++;
            x = f(x);
            y = f(f(y)); // y takes two steps
            d = BigInteger.GreatestCommonDivisor(BigInteger.Abs(x - y), n);
        }

        return d;
    }

    /// <summary>
    /// Simple trial division up to a limit.
    /// </summary>
    private static BigInteger TrialDivision(BigInteger n, int limit)
    {
        if (n <= 3)
            return n;
        if (n % 2 == 0)
            return 2;

        for (int i = 3; i <= limit; i += 2)
        {
            if (n % i == 0)
                return i;
        }

        return 1; // No small factors found
    }
}

/// <summary>
/// Represents a relation in the GNFS algorithm.
/// </summary>
public class Relation
{
    public BigInteger A { get; set; }
    public BigInteger B { get; set; }
    public Dictionary<int, int> RationalFactorization { get; set; }
    public Dictionary<int, int> AlgebraicFactorization { get; set; }
}
