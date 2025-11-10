#include "repl.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "session.hpp"
#include "bigint.hpp"
#include "help.hpp"
#include "attacks/lowe.hpp"
#include "attacks/wiener.hpp"
#include "attacks/common_modulus.hpp"
#include "attacks/fermat.hpp"
#include "attacks/rho.hpp"
#include "attacks/coppersmith.hpp"
#include "attacks/pminus1.hpp"
#include "utils/parse.hpp"

static BigInt big_from_parsed(const ParsedNumber &pn) {
    if (!pn.known) return BigInt(static_cast<uint64_t>(0));
    return BigInt(pn.raw); // hex or dec handled by constructor parse
}

// simple skeleton repl loop
int repl_main() {
    SessionState session; // currently empty
    std::cout << "repl running (type 'help' for help)\n";
    std::string line;
    for (;;) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        if (line == "hi") {
            std::cout <<
                    "hello\n";
            continue;
        }

        // handle help and help <command>
        if (line == "help" || line.substr(0, 5) == "help ") {
            if (line == "help") {
                help::print_general();
            } else {
                // extract command after "help "
                std::string cmd = line.substr(5);
                // trim leading/trailing whitespace
                size_t start = cmd.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    cmd = cmd.substr(start);
                    size_t end = cmd.find_last_not_of(" \t");
                    if (end != std::string::npos) {
                        cmd = cmd.substr(0, end + 1);
                    }
                }
                help::print_command_help(cmd);
            }
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
            BigInt m(static_cast<uint64_t>(42));
            unsigned e = 3;
            BigInt n1("0x1000003f");
            BigInt n2("0x1000005b");
            BigInt n3("0x1000006f");
            BigInt c1 = BigInt::powm(m, BigInt(static_cast<uint64_t>(e)), n1);
            BigInt c2 = BigInt::powm(m, BigInt(static_cast<uint64_t>(e)), n2);
            BigInt c3 = BigInt::powm(m, BigInt(static_cast<uint64_t>(e)), n3);
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
            BigInt one(static_cast<uint64_t>(1));
            BigInt n = p * q;
            BigInt phi = (p - one) * (q - one);
            unsigned small_ds[] = {5, 7, 11, 13, 17, 19, 23, 29, 37, 41, 43, 47};
            BigInt d(static_cast<uint64_t>(0));
            for (unsigned cand: small_ds) {
                BigInt cd{static_cast<uint64_t>(cand)}; // not a function decl
                if (BigInt::gcd(cd, phi) == BigInt(static_cast<uint64_t>(1))) {
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
                    if (BigInt::gcd(cd, phi) == BigInt(static_cast<uint64_t>(1))) {
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
            BigInt one(static_cast<uint64_t>(1));
            BigInt phi = (p - one) * (q - one);
            BigInt m(static_cast<uint64_t>(12345));
            BigInt e1(static_cast<uint64_t>(17));
            BigInt e2(static_cast<uint64_t>(13));
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
            std::cout << "enter N> ";
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
        if (line == "rho") {
            std::cout << "enter N> ";
            std::string n_in;
            std::getline(std::cin, n_in);
            auto n_parsed = utils::parse_number_adv(n_in);
            if (!n_parsed.known) {
                std::cout << "invalid N (need dec or 0x..)\n";
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
            try {
                BigInt n = big_from_parsed(n_parsed);
                RhoResult r = rho_attack(n, iters);
                if (r.success) {
                    std::cout << "rho factor: " << r.factor.to_dec() << " (" << r.factor.to_hex() << ")\n";
                } else {
                    std::cout << "rho failed: " << r.log << "\n";
                }
            } catch (const std::exception &ex) {
                std::cout << "parse error: " << ex.what() << "\n";
            }
            continue;
        }
        if (line == "coppersmith") {
            std::cout << "enter type (1=linear, 2=partial-msg)> ";
            std::string type_in;
            std::getline(std::cin, type_in);
            auto type_p = utils::parse_number_adv(type_in);
            if (!type_p.known) {
                std::cout << "invalid type\n";
                continue;
            }
            unsigned type = static_cast<unsigned>(std::stoull(type_p.raw, nullptr, type_p.is_dec ? 10 : 16));

            if (type == 1) {
                // linear: ax + b ≡ 0 (mod n)
                std::cout << "enter a (coeff of x)> ";
                std::string a_in;
                std::getline(std::cin, a_in);
                auto a_p = utils::parse_number_adv(a_in);
                if (!a_p.known) {
                    std::cout << "bad a\n";
                    continue;
                }
                std::cout << "enter b (constant)> ";
                std::string b_in;
                std::getline(std::cin, b_in);
                auto b_p = utils::parse_number_adv(b_in);
                if (!b_p.known) {
                    std::cout << "bad b\n";
                    continue;
                }
                std::cout << "enter n (modulus)> ";
                std::string n_in;
                std::getline(std::cin, n_in);
                auto n_p = utils::parse_number_adv(n_in);
                if (!n_p.known) {
                    std::cout << "bad n\n";
                    continue;
                }
                try {
                    BigInt a = big_from_parsed(a_p);
                    BigInt b = big_from_parsed(b_p);
                    BigInt n = big_from_parsed(n_p);
                    auto res = coppersmith_univariate_linear(a, b, n);
                    if (res.success) {
                        std::cout << "coppersmith success: x=" << res.root.to_hex() << " (dec=" << res.root.to_dec() << ")\n";
                    } else {
                        std::cout << "coppersmith failed: " << res.log << "\n";
                    }
                } catch (const std::exception &ex) {
                    std::cout << "error: " << ex.what() << "\n";
                }
            } else if (type == 2) {
                // partial message recovery
                std::cout << "enter c (ciphertext)> ";
                std::string c_in;
                std::getline(std::cin, c_in);
                auto c_p = utils::parse_number_adv(c_in);
                if (!c_p.known) {
                    std::cout << "bad c\n";
                    continue;
                }
                std::cout << "enter e (exponent, typically 3 or 5)> ";
                std::string e_in;
                std::getline(std::cin, e_in);
                auto e_p = utils::parse_number_adv(e_in);
                if (!e_p.known) {
                    std::cout << "bad e\n";
                    continue;
                }
                std::cout << "enter n (modulus)> ";
                std::string n_in;
                std::getline(std::cin, n_in);
                auto n_p = utils::parse_number_adv(n_in);
                if (!n_p.known) {
                    std::cout << "bad n\n";
                    continue;
                }
                std::cout << "enter m_high (known high bits of message)> ";
                std::string mh_in;
                std::getline(std::cin, mh_in);
                auto mh_p = utils::parse_number_adv(mh_in);
                if (!mh_p.known) {
                    std::cout << "bad m_high\n";
                    continue;
                }
                std::cout << "enter unknown_bits (number of unknown low bits)> ";
                std::string ub_in;
                std::getline(std::cin, ub_in);
                auto ub_p = utils::parse_number_adv(ub_in);
                if (!ub_p.known) {
                    std::cout << "bad unknown_bits\n";
                    continue;
                }
                try {
                    BigInt c = big_from_parsed(c_p);
                    unsigned e = static_cast<unsigned>(std::stoull(e_p.raw, nullptr, e_p.is_dec ? 10 : 16));
                    BigInt n = big_from_parsed(n_p);
                    BigInt m_high = big_from_parsed(mh_p);
                    size_t unknown_bits = static_cast<size_t>(std::stoull(ub_p.raw, nullptr, ub_p.is_dec ? 10 : 16));

                    auto res = coppersmith_small_e_partial_msg(c, e, n, m_high, unknown_bits);
                    if (res.success) {
                        std::cout << "coppersmith success: full_message=" << res.root.to_hex() << " (dec=" << res.root.to_dec() << ")\n";
                    } else {
                        std::cout << "coppersmith failed: " << res.log << "\n";
                    }
                } catch (const std::exception &ex) {
                    std::cout << "error: " << ex.what() << "\n";
                }
            } else {
                std::cout << "unknown type (use 1 or 2)\n";
            }
            continue;
        }
        if (line == "coppersmith-selftest") {
            // selftest scenario: partial message recovery with e=3
            std::cout << "running coppersmith selftest for partial message recovery...\n";

            // setup: small primes for testing
            BigInt p("1000003");
            BigInt q("1000033");
            BigInt n = p * q;
            BigInt one(static_cast<uint64_t>(1));
            BigInt phi = (p - one) * (q - one);
            unsigned e = 3;

            // create a message where we "know" the high bits
            // message = 0xABCD1234, we'll pretend we know 0xABCD0000 and search for 0x1234
            BigInt full_msg(static_cast<uint64_t>(0xABCD1234));
            size_t unknown_bits = 16;
            BigInt two(static_cast<uint64_t>(2));
            BigInt shift_amount(static_cast<uint64_t>(1));
            for (size_t i = 0; i < unknown_bits; i++) {
                shift_amount *= two;
            }
            BigInt mask = shift_amount - one;
            BigInt m_high = full_msg - (full_msg % shift_amount);

            // encrypt
            BigInt c = BigInt::powm(full_msg, BigInt(static_cast<uint64_t>(e)), n);

            std::cout << "setup: n=" << n.to_hex() << " e=" << e << "\n";
            std::cout << "full_msg=" << full_msg.to_hex() << " (hiding low " << unknown_bits << " bits)\n";
            std::cout << "m_high=" << m_high.to_hex() << " c=" << c.to_hex() << "\n";

            // run attack
            auto res = coppersmith_small_e_partial_msg(c, e, n, m_high, unknown_bits);
            if (res.success && res.root == full_msg) {
                std::cout << "coppersmith selftest SUCCESS! recovered=" << res.root.to_hex() << "\n";
            } else {
                std::cout << "coppersmith selftest FAILED: expected=" << full_msg.to_hex();
                if (res.success) {
                    std::cout << " got=" << res.root.to_hex();
                } else {
                    std::cout << " (no root found)";
                }
                std::cout << " log=" << res.log << "\n";
            }
            continue;
        }
        if (line == "pminus1") {
            std::cout << "enter N> ";
            std::string n_in; std::getline(std::cin, n_in);
            auto n_p = utils::parse_number_adv(n_in);
            if(!n_p.known) { std::cout << "bad n\n"; continue; }
            std::cout << "enter B1 (stage1 bound, dec default 100000)> ";
            std::string b1_in; std::getline(std::cin, b1_in);
            unsigned long long B1 = 100000ULL;
            if(!b1_in.empty()) {
                auto b1_p = utils::parse_number_adv(b1_in);
                if(b1_p.known && b1_p.is_dec) B1 = std::stoull(b1_p.raw); }
            std::cout << "enter B2 (stage2 bound, dec; 0 to disable, default 0)> ";
            std::string b2_in; std::getline(std::cin, b2_in);
            unsigned long long B2 = 0ULL;
            if(!b2_in.empty()) {
                auto b2_p = utils::parse_number_adv(b2_in);
                if(b2_p.known && b2_p.is_dec) B2 = std::stoull(b2_p.raw); }
            std::cout << "enter trials (bases to try, dec default 5)> ";
            std::string t_in; std::getline(std::cin, t_in);
            unsigned long long trials = 5ULL;
            if(!t_in.empty()) {
                auto t_p = utils::parse_number_adv(t_in);
                if(t_p.known && t_p.is_dec) trials = std::stoull(t_p.raw); }
            try {
                BigInt n = big_from_parsed(n_p);
                PMinus1Result pr = pollards_pminus1(n, B1, trials, B2);
                if(pr.success) {
                    std::cout << "p-1 factor: " << pr.factor.to_dec() << " (" << pr.factor.to_hex() << ")\n";
                } else {
                    std::cout << "p-1 failed: " << pr.log << "\n";
                }
            } catch(const std::exception &ex) { std::cout << "error: " << ex.what() << "\n"; }
            continue;
        }
        if (line == "pminus1-selftest") {
            // choose primes where p-1 is very smooth and a stage2 window can still work
            BigInt p("1000003"); // p-1 = 2*3*167*997
            BigInt q("1000033");
            BigInt n = p * q;
            unsigned long long B1 = 100000ULL;
            unsigned long long B2 = 1000000ULL; // allow stage2 if needed
            PMinus1Result pr = pollards_pminus1(n, B1, 5ULL, B2);
            if(pr.success) {
                std::cout << "p-1 success factor=" << pr.factor.to_dec() << " (" << pr.factor.to_hex() << ")\n";
            } else {
                std::cout << "p-1 selftest failed: " << pr.log << "\n";
            }
            continue;
        }
    }
    return 0;
}
