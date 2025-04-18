using System.Globalization;
using System.Numerics;

// ReSharper disable InconsistentNaming

namespace rsaShit.Core
{
    public class RSAState
    {
        public BigInteger? e;
        public BigInteger? d;
        public BigInteger? N;
        public BigInteger? p;
        public BigInteger? q;
        public BigInteger? phi;

        public static RSAState FromArgs(string[] args)
        {
            var state = new RSAState();
            var argDict = ParseArgs(args);

            foreach (var kv in argDict)
            {
                string key = kv.Key.ToLower();
                BigInteger value = ParseBigInt(kv.Value);

                switch (key)
                {
                    case "-e":
                        state.e = value;
                        break;
                    case "-d":
                        state.d = value;
                        break;
                    case "-n":
                        state.N = value;
                        break;
                    case "-p":
                        state.p = value;
                        break;
                    case "-q":
                        state.q = value;
                        break;
                    case "-phi":
                        state.phi = value;
                        break;
                }
            }

            return state;
        }

        public static Dictionary<string, string> ParseArgs(string[] args)
        {
            var result = new Dictionary<string, string>();

            for (int i = 0; i < args.Length - 1; i++)
            {
                if (args[i].StartsWith("-"))
                {
                    result[args[i]] = args[i + 1];
                    i++; // skip next one since it's the value
                }
            }

            return result;
        }

        public static BigInteger ParseBigInt(string input)
        {
            // Auto-detect hex
            if (input.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
                return BigInteger.Parse(input.Substring(2), NumberStyles.HexNumber);

            // Treat as unsigned decimal number
            if (input.All(char.IsDigit))
                return BigInteger.Parse(input, NumberStyles.Integer, CultureInfo.InvariantCulture);

            // Fallback for other formats
            return BigInteger.Parse(input);
        }

        public void PrintKnownValues(string attackName = "")
        {
            Console.WriteLine(
                attackName.Length > 0 ? $"\nValues from '{attackName}':" : "\nKnown RSA values:"
            );
            // ReSharper disable once NotAccessedVariable
            int knownCount = 0;

            if (e.HasValue)
            {
                Console.WriteLine($"  e    = {e}");
                knownCount++;
            }
            if (d.HasValue)
            {
                Console.WriteLine($"  d    = {d}");
                knownCount++;
            }
            if (N.HasValue)
            {
                Console.WriteLine($"  N    = {N}");
                knownCount++;
            }
            if (p.HasValue)
            {
                Console.WriteLine($"  p    = {p}");
                knownCount++;
            }
            if (q.HasValue)
            {
                Console.WriteLine($"  q    = {q}");
                knownCount++;
            }
            if (phi.HasValue)
            {
                Console.WriteLine($"  phi  = {phi}");
                knownCount++;
            }

            /*Console.WriteLine(
                knownCount == 0
                    ? $"{Color.Red}What do you even want this to do?"
                    : $"Total of: {Color.Green}{knownCount}{Color.Reset}"
            );*/
        }

        public bool Has(string name)
        {
            return name.ToLower() switch
            {
                "e" => e.HasValue,
                "d" => d.HasValue,
                "n" => N.HasValue,
                "p" => p.HasValue,
                "q" => q.HasValue,
                "phi" => phi.HasValue,
                _ => false,
            };
        }

        public bool Has(params string[] names)
        {
            foreach (var name in names)
                if (!Has(name))
                    return false;
            return true;
        }

        public static RSAState Interactive()
        {
            var state = new RSAState();
            Console.WriteLine(
                "Enter RSA values (e, d, N, p, q, phi) one by one. Type 'done' to finish."
            );

            while (true)
            {
                Console.Write("> ");
                string input = Console.ReadLine();

                if (input?.ToLower() == "done")
                    break;

                string[] parts = input.Split('=');
                if (parts.Length == 2)
                {
                    string key = parts[0].Trim().ToLower();
                    string value = parts[1].Trim();

                    BigInteger parsedValue = ParseBigInt(value);

                    switch (key)
                    {
                        case "e":
                            state.e = parsedValue;
                            Console.WriteLine($"e is {parsedValue}");
                            break;
                        case "d":
                            state.d = parsedValue;
                            Console.WriteLine($"d is {parsedValue}");
                            break;
                        case "n":
                            state.N = parsedValue;
                            Console.WriteLine($"N is {parsedValue}");
                            break;
                        case "p":
                            state.p = parsedValue;
                            Console.WriteLine($"p is {parsedValue}");
                            break;
                        case "q":
                            state.q = parsedValue;
                            Console.WriteLine($"q is {parsedValue}");
                            break;
                        case "phi":
                            state.phi = parsedValue;
                            Console.WriteLine($"phi is {parsedValue}");
                            break;
                        default:
                            Console.WriteLine($"Unknown key: {key}");
                            break;
                    }
                }
                else
                {
                    Console.WriteLine("Format should be 'key=value'.");
                }
            }

            return state;
        }
    }
}
