#include <iostream>
#include "src/repl.hpp"

int main() {
    // cool figlet banner
    std::cout <<
            "\n:::::::..   .::::::.   :::.     .::::::.   ::   .:  :::::::::::::::\r\n;;;;``;;;; ;;;`    `   ;;`;;   ;;;`    `  ,;;   ;;, ;;;;;;;;;;;''''\r\n [[[,/[[[' '[==/[[[[, ,[[ '[[, '[==/[[[[,,[[[,,,[[[ [[[     [[     \r\n $$$$$$c     '''    $c$$$cc$$$c  '''    $\"$$$\"\"\"$$$ $$$     $$     \r\n 888b \"88bo,88b    dP 888   888,88b    dP 888   \"88o888     88,    \r\n MMMM   \"W\"  \"YMmMY\"  YMM   \"\"`  \"YMmMY\"  MMM    YMMMMM     MMM    \r\n\n";
    std::cout << "Proving that RSA is \e[1mshit\e[0m\n\n";
    return repl_main();
}
