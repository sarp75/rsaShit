#include "parse.hpp"
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace utils {
    std::optional<std::string> parse_number(const std::string &s) {
        if(s.empty()) return std::nullopt;
        // very naive stub: accept hex starting 0x or all digits
        if(s.size() > 2 && s[0]=='0' && (s[1]=='x' || s[1]=='X')) return s;
        for(char c: s) if(!std::isdigit(static_cast<unsigned char>(c))) return std::nullopt;
        return s;
    }

    static std::string trim(const std::string &in) {
        size_t a=0,b=in.size();
        while(a<b && std::isspace((unsigned char)in[a])) a++;
        while(b>a && std::isspace((unsigned char)in[b-1])) b--;
        return in.substr(a,b-a);
    }

    ParsedNumber parse_number_adv(const std::string &s_in) {
        ParsedNumber pn; pn.raw = trim(s_in);
        if(pn.raw.empty() || pn.raw == "idk") { pn.known = false; return pn; }
        // file:path support (load contents)
        if(pn.raw.rfind("file:",0)==0) {
            std::string path = pn.raw.substr(5);
            std::ifstream f(path);
            if(!f.good()) { pn.known=false; return pn; }
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            pn.raw = trim(content);
        }
        // hex
        if(pn.raw.size()>2 && pn.raw[0]=='0' && (pn.raw[1]=='x'||pn.raw[1]=='X')) {
            bool ok=true; for(size_t i=2;i<pn.raw.size();++i){ char c=pn.raw[i]; if(!std::isxdigit((unsigned char)c)) { ok=false; break; } }
            if(ok) { pn.known=true; pn.is_hex=true; return pn; }
        }
        // decimal
        bool dec=true; for(char c: pn.raw){ if(!std::isdigit((unsigned char)c)) { dec=false; break; } }
        if(dec) { pn.known=true; pn.is_dec=true; return pn; }
        pn.known=false; return pn;
    }
}
