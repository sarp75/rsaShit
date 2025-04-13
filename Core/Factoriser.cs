namespace rsaShit.Core;


using System;
using System.Numerics;
using System.Net.Http;
using System.Threading.Tasks;
using System.Text.RegularExpressions;

public static class Factoriser
{
    private static readonly HttpClient HttpClient = new HttpClient();
    private const string FactorDbApiUrl = "http://factordb.com/api";

    /// <summary>
    /// Factorises a large integer into its prime factors using FactorDB API
    /// </summary>
    /// <param name="value">The BigInteger to be factorized</param>
    /// <returns>A tuple containing the two prime factors (p, q)</returns>
    public static (BigInteger, BigInteger) Factorise(BigInteger value)
    {
        Console.WriteLine($"[*] Attempting to factorize: {value}");

        // Try to factorize using FactorDB API
        var result = FactoriseUsingFactorDbApi(value).GetAwaiter().GetResult();

        if (result.Item1 != 0 && result.Item2 != 0)
        {
            Console.WriteLine("[+] Successfully factorized using FactorDB API");
            return result;
        }

        // If FactorDB didn't work, try local methods for small numbers
        if (value < BigInteger.Pow(2, 64))
        {
            Console.WriteLine("[*] FactorDB failed, attempting local small factorization...");
            return FactoriseSmallNumber(value);
        }

        // If all methods fail, throw an exception
        throw new Exception("Failed to factorize the number. It might be too large or prime.");
    }

    /// <summary>
    /// Attempts to factorize a number using the FactorDB API
    /// </summary>
    private static async Task<(BigInteger, BigInteger)> FactoriseUsingFactorDbApi(BigInteger value)
    {
        try
        {
            Console.WriteLine("[*] Querying FactorDB API...");
            string url = $"{FactorDbApiUrl}?query={value}";

            HttpResponseMessage response = await HttpClient.GetAsync(url);
            if (!response.IsSuccessStatusCode)
            {
                Console.WriteLine($"[!] FactorDB API request failed: {response.StatusCode}");
                return (0, 0);
            }

            string jsonResponse = await response.Content.ReadAsStringAsync();

            // Simple JSON parsing using regex (in a real-world scenario, you might want to use a proper JSON library)
            string statusPattern = "\"status\"\\s*:\\s*\"([^\"]+)\"";
            string factorsPattern = "\"factors\"\\s*:\\s*\\[\\s*(\\[.+?\\])\\s*\\]";

            Match statusMatch = Regex.Match(jsonResponse, statusPattern);
            Match factorsMatch = Regex.Match(jsonResponse, factorsPattern);

            if (!statusMatch.Success || !factorsMatch.Success)
            {
                Console.WriteLine("[!] Failed to parse FactorDB API response");
                return (0, 0);
            }

            string status = statusMatch.Groups[1].Value;

            // Status "FF" means fully factored
            if (status != "FF")
            {
                Console.WriteLine($"[!] Number is not fully factored in FactorDB. Status: {status}");
                return (0, 0);
            }

            // Parse factors
            string factorsJson = factorsMatch.Groups[1].Value;
            var factorsList = ParseFactors(factorsJson);

            // For RSA, we need exactly two prime factors
            if (factorsList.Count == 2)
            {
                return (factorsList[0], factorsList[1]);
            }
            else if (factorsList.Count > 2)
            {
                // If we have more than 2 factors, combine them appropriately for RSA
                Console.WriteLine($"[!] Found {factorsList.Count} factors instead of 2. Attempting to combine them.");
                return CombineFactorsForRsa(factorsList, value);
            }
            else
            {
                Console.WriteLine("[!] Not enough factors found in FactorDB");
                return (0, 0);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[!] Error accessing FactorDB API: {ex.Message}");
            return (0, 0);
        }
    }

    /// <summary>
    /// Parse the factors from FactorDB JSON response
    /// </summary>
    private static List<BigInteger> ParseFactors(string factorsJson)
    {
        var result = new List<BigInteger>();
        var factorPairs = Regex.Matches(factorsJson, "\\[(\\d+),(\\d+)\\]");

        foreach (Match match in factorPairs)
        {
            if (match.Groups.Count >= 3)
            {
                string factor = match.Groups[1].Value;
                string exponent = match.Groups[2].Value;

                BigInteger factorValue = BigInteger.Parse(factor);
                int exponentValue = int.Parse(exponent);

                // Add the factor exponent times
                for (int i = 0; i < exponentValue; i++)
                {
                    result.Add(factorValue);
                }
            }
        }

        return result;
    }

    /// <summary>
    /// Combine multiple factors to get two factors for RSA (p and q)
    /// </summary>
    private static (BigInteger, BigInteger) CombineFactorsForRsa(List<BigInteger> factors, BigInteger n)
    {
        // For RSA, we typically need two large prime factors
        // If we have more factors, we need to combine them appropriately

        // Try different combinations to find p and q where p*q = n
        BigInteger p = 1;
        BigInteger q = 1;

        // Try a simple approach: put half of the factors into p, half into q
        int midPoint = factors.Count / 2;

        for (int i = 0; i < factors.Count; i++)
        {
            if (i < midPoint)
                p *= factors[i];
            else
                q *= factors[i];
        }

        // Verify p*q = n
        if (p * q == n)
            return (p, q);

        // If simple approach didn't work, try a brute force approach
        // This is a simplified implementation and might not work for all cases
        for (int mask = 1; mask < (1 << factors.Count) - 1; mask++)
        {
            BigInteger candidateP = 1;
            BigInteger candidateQ = 1;

            for (int i = 0; i < factors.Count; i++)
            {
                if ((mask & (1 << i)) != 0)
                    candidateP *= factors[i];
                else
                    candidateQ *= factors[i];
            }

            if (candidateP * candidateQ == n)
                return (candidateP, candidateQ);
        }

        // If all combinations fail
        throw new Exception("Could not find appropriate p and q from the factors");
    }

    /// <summary>
    /// Trial division factorization for smaller numbers
    /// </summary>
    private static (BigInteger, BigInteger) FactoriseSmallNumber(BigInteger n)
    {
        Console.WriteLine("[*] Attempting trial division for smaller number...");

        // Check if the number is even
        if (n % 2 == 0)
            return (2, n / 2);

        BigInteger limit = Sqrt(n);

        // Try odd divisors
        for (BigInteger i = 3; i <= limit; i += 2)
        {
            if (n % i == 0)
                return (i, n / i);
        }

        // If we reach here, n is prime
        throw new Exception("The number appears to be prime and cannot be factorized");
    }

    /// <summary>
    /// Calculate integer square root for BigInteger
    /// </summary>
    private static BigInteger Sqrt(BigInteger n)
    {
        if (n == 0) return 0;

        BigInteger x = n;
        BigInteger y = (x + 1) / 2;

        while (y < x)
        {
            x = y;
            y = (x + n / x) / 2;
        }

        return x;
    }
}
