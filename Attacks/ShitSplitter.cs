using System.Numerics;
using rsaShit.Core;

namespace rsaShit.Attacks;

public class ShitSplitter : IRsaAttack
{
    public string Name => "ShitSplitter";

    // Check if we can execute this attack (we need at least e and N)
    public bool CanExecute(RSAState state)
    {
        return state.Has("e", "N") && !state.Has("p", "q", "d", "phi");
    }

    // Execute the attack to find the missing RSA values
    public void Execute(RSAState state)
    {
        Console.WriteLine($"[*] Executing {Name}...");

        if (!state.e.HasValue || !state.N.HasValue)
        {
            Console.WriteLine("[!] Error: Both 'e' and 'N' are required for this attack.");
            return;
        }

        BigInteger e = state.e.Value;
        // ReSharper disable once InconsistentNaming
        BigInteger N = state.N.Value;

        // Step 1: Factor N to find p and q
        Console.WriteLine("[*] Factorizing N to find p and q...");
        try
        {
            (BigInteger p, BigInteger q) = Factoriser.Factorise(N);

            state.p = p;
            state.q = q;
            Console.WriteLine(Color.Green + $"[+] Found p = {p}");
            Console.WriteLine($"[+] Found q = {q}" + Color.Reset);

            // Verify p * q = N
            if (p * q != N)
            {
                Console.WriteLine(
                    Color.Red + "[!] Error: p * q does not equal N. Check your shit." + Color.Reset
                );
                return;
            }

            // Step 2: Calculate phi(N) = (p-1)(q-1)
            BigInteger phi = (p - 1) * (q - 1);
            state.phi = phi;
            Console.WriteLine(Color.Green + $"[+] Calculated phi(N) = {phi}");

            // Step 3: Calculate d = e^(-1) mod phi(N)
            BigInteger d = ModInverse(e, phi);
            state.d = d;
            Console.WriteLine($"[+] Calculated d = {d}" + Color.Reset);

            // Verify e*d ≡ 1 (mod phi(N))
            BigInteger verification = (e * d) % phi;
            if (verification != 1)
            {
                Console.WriteLine(
                    $"[!] Warning: Verification failed. e*d ≡ {verification} (mod phi(N)), expected 1"
                );
            }
            else
            {
                Console.WriteLine("[+] Verification successful: e*d ≡ 1 (mod phi(N))");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[!] Attack failed: {ex.Message}");
            Console.WriteLine("[?] Force trial factorization? (y/N): ");
            string input = Console.ReadLine()!;
            if (input.ToLower() == "y")
            {
                try
                {
                    var (a, b) = Factoriser.FactoriseSmallNumber(N);
                    state.p = a;
                    state.q = b;
                }
                catch (Exception exception)
                {
                    Console.WriteLine(exception);
                    throw;
                }
            }
            else
            {
                Console.WriteLine("[!] Exiting attack.");
            }
        }
    }

    // Calculate modular multiplicative inverse using extended Euclidean algorithm
    private BigInteger ModInverse(BigInteger a, BigInteger m)
    {
        if (m == 1)
            return 0;

        BigInteger m0 = m;
        BigInteger y = 0,
            x = 1;

        while (a > 1)
        {
            BigInteger q = a / m;
            BigInteger t = m;

            m = a % m;
            a = t;
            t = y;

            y = x - q * y;
            x = t;
        }

        if (x < 0)
            x += m0;

        return x;
    }
}
