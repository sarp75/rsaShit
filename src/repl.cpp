#include "repl.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "session.hpp"
#include "bigint.hpp"
#include "attacks/lowe.hpp"
#include "attacks/wiener.hpp"
#include "attacks/common_modulus.hpp"
#include "attacks/fermat.hpp"
#include "utils/parse.hpp"

static BigInt big_from_parsed(const ParsedNumber &pn) {
    if (!pn.known) return BigInt(uint64_t(0));
    return BigInt(pn.raw); // hex or dec handled by constructor parse
}

// simple skeleton repl loop
int repl_main() {
    SessionState session; // currently empty
    std::cout << "rsaShit minimal skeleton repl (type 'help' for help)\n";
    std::string line;
    for (;;) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        if (line == "hi") {
            std::cout <<
                    "hello\n";
            continue;
        }
        if (line == "help") {
            std::cout <<
                    "commands: help, quit, show, lowe, wiener, cmod, fermat, hi\n";
            continue;
        }
        if (line == "show") {
            std::cout << "session items: " << session.size() << " (stub)\n";
            continue;
        }
        if (line == "lowe") {
            std::cout << "enter e> ";
            std::string e_in;
            std::getline(std::cin, e_in);
            auto e_parsed = utils::parse_number_adv(e_in);
            if (!e_parsed.known) {
                std::cout << "invalid e (need dec or 0x..)\n";
                continue;
            }
            unsigned e_val = static_cast<unsigned>(std::stoull(e_parsed.raw, nullptr, e_parsed.is_hex ? 16 : 10));
            std::cout << "enter count of targets (>= e)> ";
            std::string ct_in;
            std::getline(std::cin, ct_in);
            auto ct_parsed = utils::parse_number_adv(ct_in);
            if (!ct_parsed.known) {
                std::cout << "invalid count\n";
                continue;
            }
            size_t count = static_cast<size_t>(std::stoull(ct_parsed.raw, nullptr, ct_parsed.is_hex ? 16 : 10));
            if (count < e_val) {
                std::cout << "need at least e targets\n";
                continue;
            }
            std::vector<LoweTarget> targets;
            for (size_t i = 0; i < count; i++) {
                std::cout << "enter N[" << i << "]> ";
                std::string n_in;
                std::getline(std::cin, n_in);
                auto n_p = utils::parse_number_adv(n_in);
                if (!n_p.known) {
                    std::cout << "bad N; aborting lowe\n";
                    targets.clear();
                    break;
                }
                std::cout << "enter C[" << i << "]> ";
                std::string c_in;
                std::getline(std::cin, c_in);
                auto c_p = utils::parse_number_adv(c_in);
                if (!c_p.known) {
                    std::cout << "bad C; aborting lowe\n";
                    targets.clear();
                    break;
                }
                try {
                    targets.push_back(LoweTarget{big_from_parsed(n_p), big_from_parsed(c_p)});
                } catch (const std::exception &ex) {
                    std::cout << "parse error: " << ex.what() << "\n";
                    targets.clear();
                    break;
                }
            }
            if (targets.empty()) continue;
            LoweResult r = low_e_broadcast(targets, e_val);
            if (r.success) {
                std::cout << "low-e recovered m: " << r.m.to_dec() << " (" << r.m.to_hex() << ")\n";
            } else {
                std::cout << "low-e failed: " << r.log << "\n";
            }
            continue;
        }
        if (line == "lowe-demo") {
            // construct tiny demo: plaintext m=42, e=3, small-ish coprime moduli
            BigInt m(uint64_t(42));
            unsigned e = 3;
            BigInt n1("0x1000003f");
            BigInt n2("0x1000005b");
            BigInt n3("0x1000006f");
            BigInt c1 = BigInt::powm(m, BigInt(uint64_t(e)), n1);
            BigInt c2 = BigInt::powm(m, BigInt(uint64_t(e)), n2);
            BigInt c3 = BigInt::powm(m, BigInt(uint64_t(e)), n3);
            std::vector<LoweTarget> t{LoweTarget{n1, c1}, LoweTarget{n2, c2}, LoweTarget{n3, c3}};
            LoweResult r = low_e_broadcast(t, e);
            if (r.success) {
                std::cout << "low-e recovered m: " << r.m.to_dec() << " (" << r.m.to_hex() << ")\n";
            } else {
                std::cout << "low-e failed: " << r.log << "\n";
            }
            continue;
        }
        if (line == "wiener") {
            std::cout << "enter N> ";
            std::string n_in;
            std::getline(std::cin, n_in);
            auto n_p = utils::parse_number_adv(n_in);
            if (!n_p.known) {
                std::cout << "invalid N\n";
                continue;
            }
            std::cout << "enter e> ";
            std::string e_in;
            std::getline(std::cin, e_in);
            auto e_p = utils::parse_number_adv(e_in);
            if (!e_p.known) {
                std::cout << "invalid e\n";
                continue;
            }
            try {
                BigInt n(big_from_parsed(n_p));
                BigInt e(big_from_parsed(e_p));
                WienerResult wr = wiener_attack(n, e);
                if (wr.success) {
                    std::cout << "wiener success: p=" << wr.p.to_hex() << ", q=" << wr.q.to_hex() << ", d=" << wr.d.
                            to_hex() << "\n";
                } else {
                    std::cout << "wiener failed: " << wr.log << "\n";
                }
            } catch (const std::exception &ex) {
                std::cout << "parse/attack error: " << ex.what() << "\n";
            }
            continue;
        }
        if (line == "wiener-selftest") {
            // selftest: construct rsa with deliberately small d so wiener should recover it
            // shit primes set 1
            BigInt p("1000003");
            BigInt q("1000033");
            BigInt one(uint64_t(1));
            BigInt n = p * q;
            BigInt phi = (p - one) * (q - one);
            unsigned small_ds[] = {5, 7, 11, 13, 17, 19, 23, 29, 37, 41, 43, 47};
            BigInt d(uint64_t(0));
            for (unsigned cand: small_ds) {
                BigInt cd{static_cast<uint64_t>(cand)}; // not a function decl
                if (BigInt::gcd(cd, phi) == BigInt(uint64_t(1))) {
                    d = cd;
                    break;
                }
            }
            if (d.is_zero()) {
                std::cout << "selftest: first prime set failed, trying fallback primes\n";
                // fallback primes (ensure coprime phi) shit
                p = BigInt("10007");
                q = BigInt("10009");
                n = p * q;
                phi = (p - one) * (q - one);
                for (unsigned cand: small_ds) {
                    BigInt cd{static_cast<uint64_t>(cand)};
                    if (BigInt::gcd(cd, phi) == BigInt(uint64_t(1))) {
                        d = cd;
                        break;
                    }
                }
            }
            if (d.is_zero()) {
                std::cout << "selftest: failed to find small coprime d\n";
                continue;
            }
            auto inv = BigInt::mod_inverse(d, phi);
            if (!inv) {
                std::cout << "selftest: mod inverse failed for d\n";
                continue;
            }
            BigInt e = *inv; // e*d ≡ 1 mod phi
            std::cout << "selftest N=" << n.to_dec() << " (hex=" << n.to_hex() << ") e=" << e.to_dec() << " d=" << d.
                    to_dec() << "\n";
            // run wiener
            WienerResult wr = wiener_attack(n, e);
            if (wr.success) {
                std::cout << "wiener success: p=" << wr.p.to_dec() << ", q=" << wr.q.to_dec() << ", d=" << wr.d.to_dec()
                        << "\n";
            } else {
                std::cout << "wiener failed: " << wr.log << "\n";
                // diagnostic: check actual relation k = (e*d-1)/phi
                BigInt k = (e * d - one) / phi;
                std::cout << "diag: k=" << k.to_dec() << " phi=" << phi.to_dec() << " n^(1/4)≈" <<
                        BigInt::nth_root_floor(n, 4).to_dec() << "\n";
            }
            continue;
        }
        if (line == "cmod") {
            std::cout << "enter n> ";
            std::string n_in;
            std::getline(std::cin, n_in);
            auto n_p = utils::parse_number_adv(n_in);
            if (!n_p.known) {
                std::cout << "bad n\n";
                continue;
            }
            std::cout << "enter e1> ";
            std::string e1_in;
            std::getline(std::cin, e1_in);
            auto e1_p = utils::parse_number_adv(e1_in);
            if (!e1_p.known) {
                std::cout << "bad e1\n";
                continue;
            }
            std::cout << "enter e2> ";
            std::string e2_in;
            std::getline(std::cin, e2_in);
            auto e2_p = utils::parse_number_adv(e2_in);
            if (!e2_p.known) {
                std::cout << "bad e2\n";
                continue;
            }
            std::cout << "enter c1> ";
            std::string c1_in;
            std::getline(std::cin, c1_in);
            auto c1_p = utils::parse_number_adv(c1_in);
            if (!c1_p.known) {
                std::cout << "bad c1\n";
                continue;
            }
            std::cout << "enter c2> ";
            std::string c2_in;
            std::getline(std::cin, c2_in);
            auto c2_p = utils::parse_number_adv(c2_in);
            if (!c2_p.known) {
                std::cout << "bad c2\n";
                continue;
            }
            try {
                BigInt n(big_from_parsed(n_p));
                BigInt e1(big_from_parsed(e1_p));
                BigInt e2(big_from_parsed(e2_p));
                BigInt c1(big_from_parsed(c1_p));
                BigInt c2(big_from_parsed(c2_p));
                auto res = common_modulus_attack(n, e1, e2, c1, c2);
                if (res.success)
                    std::cout << "common modulus success m=" << res.m.to_hex() << " (dec=" << res.m.
                            to_dec() << ")\n";
                else std::cout << "common modulus failed: " << res.log << "\n";
            } catch (const std::exception &ex) { std::cout << "error: " << ex.what() << "\n"; }
            continue;
        }
        if (line == "cmod-selftest") {
            // create a small scenario: choose random m, two co-prime exponents e1,e2
            BigInt p("1000003");
            BigInt q("1000033");
            BigInt n = p * q;
            BigInt one(uint64_t(1));
            BigInt phi = (p - one) * (q - one);
            BigInt m(uint64_t(12345));
            BigInt e1(uint64_t(17));
            BigInt e2(uint64_t(13));
            if (BigInt::gcd(e1, e2) != one) {
                std::cout << "selftest exponents not coprime\n";
                continue;
            }
            BigInt c1 = BigInt::powm(m, e1, n);
            BigInt c2 = BigInt::powm(m, e2, n);
            auto cmr = common_modulus_attack(n, e1, e2, c1, c2);
            if (cmr.success && cmr.m == m) {
                std::cout << "cmod success recovered m=" << cmr.m.to_dec() << "\n";
            } else {
                std::cout << "cmod failed log=" << cmr.log << " expected=" << m.to_dec() << " got=" << cmr.m.to_dec() <<
                        "\n";
            }
            continue;
        }
        if (line == "fermat") {
            std::cout << "enter n> ";
            std::string n_in;
            std::getline(std::cin, n_in);
            auto n_p = utils::parse_number_adv(n_in);
            if (!n_p.known) {
                std::cout << "bad n\n";
                continue;
            }
            std::cout << "max iters (dec, default 1000000)> ";
            std::string it_in;
            std::getline(std::cin, it_in);
            unsigned long long iters = 1000000ULL;
            if (!it_in.empty()) {
                auto it_p = utils::parse_number_adv(it_in);
                if (it_p.known && it_p.is_dec) iters = std::stoull(it_p.raw);
            }
            FermatResult fr = fermat_factor(BigInt(big_from_parsed(n_p)), iters);
            if (fr.success) {
                std::cout << "fermat success: p=" << fr.p.to_hex() << ", q=" << fr.q.to_hex() << "\n";
            } else {
                std::cout << "fermat failed: " << fr.log << "\n";
            }
            continue;
        }
        if (line == "fermat-selftest") {
            // choose two close primes so fermat is fast
            BigInt p("1000003");
            BigInt q("1000033");
            BigInt n = p * q;
            FermatResult fr = fermat_factor(n, 100000ULL);
            if (fr.success && fr.p * fr.q == n) {
                std::cout << "fermat success: p=" << fr.p.to_dec() << ", q=" << fr.q.to_dec() << "\n";
            } else {
                std::cout << "fermat failed: " << fr.log << "\n";
            }
            continue;
        }
        std::cout << "unrecognized command (stub)\n";
    }
    return 0;
}
