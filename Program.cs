using System.Numerics;
using rsaShit.Attacks;
using rsaShit.Core;

namespace rsaShit
{
    class Program
    {
        public const string Version = "1.0.0";

        static void Main(string[] args)
        {
            Welcome();

            // Parse args into RSAState
            RSAState state;
            if (args.Length > 0)
            {
                state = RSAState.FromArgs(args);
            }
            else
            {
                state = RSAState.Interactive();
            }

            // Print what was received
            state.PrintKnownValues();

            // Load available attacks
            List<IRsaAttack> attacks = new List<IRsaAttack> { new ShitSplitter() };

            bool anySuccess = false;

            // Try all attacks that can run
            foreach (var attack in attacks)
            {
                if (attack.CanExecute(state))
                {
                    Console.WriteLine(
                        Color.Green + $"\n[>] {attack.Name} is doable..." + Color.Reset
                    );
                    attack.Execute(state);
                    state.PrintKnownValues(attack.Name); // Show updated state
                }
            }

            if (!anySuccess)
            {
                Console.WriteLine("\n[!] No attacks succeeded or were applicable.");
            }

            Console.WriteLine("\nDone.");
        }

        static void Welcome()
        {
            Console.WriteLine(
                Color.Red
                    + @"                                                                                 
                                                                                 
                                  .--.--.     ,---,               ___            
                                 /  /    '. ,--.' |      ,--,   ,--.'|_          
  __  ,-.                       |  :  /`. / |  |  :    ,--.'|   |  | :,'         
,' ,'/ /|  .--.--.              ;  |  |--`  :  :  :    |  |,    :  : ' :         
'  | |' | /  /    '    ,--.--.  |  :  ;_    :  |  |,--.`--'_  .;__,'  /          
|  |   ,'|  :  /`./   /       \  \  \    `. |  :  '   |,' ,'| |  |   |           
'  :  /  |  :  ;_    .--.  .-. |  `----.   \|  |   /' :'  | | :__,'| :           
|  | '    \  \    `.  \__\/: . .  __ \  \  |'  :  | | ||  | :   '  : |__         
;  : |     `----.   \ ,"" .--.; | /  /`--'  /|  |  ' | :'  : |__ |  | '.'|        
|  , ;    /  /`--'  //  /  ,.  |'--'.     / |  :  :_:,'|  | '.'|;  :    ;        
 ---'    '--'.     /;  :   .'   \ `--'---'  |  | ,'    ;  :    ;|  ,   /         
           `--'---' |  ,     .-./           `--''      |  ,   /  ---`-'          
                     `--`---'                           ---`-'                   
                                                                                 
"
                    + Color.Reset
            );
            Console.WriteLine($"using rsaShit v{Version}");
        }
    }
}
