#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <memory>
#include <functional>
#include <queue>
#include <stack>
#include <unordered_set>
#include <iomanip>
#include <chrono>
#include <limits>

// Forward declarations
class Decoding;
class Predicate;

// Utility functions namespace
namespace StringUtils {
    bool isPrefix(const std::string& prefix, const std::string& str) {
        if (prefix.length() > str.length()) return false;
        return str.substr(0, prefix.length()) == prefix;
    }
    
    bool isProperPrefix(const std::string& prefix, const std::string& str) {
        return isPrefix(prefix, str) && prefix.length() < str.length();
    }
    
    bool isSuffix(const std::string& suffix, const std::string& str) {
        if (suffix.length() > str.length()) return false;
        return str.substr(str.length() - suffix.length()) == suffix;
    }
    
    bool isProperSuffix(const std::string& suffix, const std::string& str) {
        return isSuffix(suffix, str) && suffix.length() < str.length();
    }
    
    bool isInfix(const std::string& infix, const std::string& str) {
        return str.find(infix) != std::string::npos;
    }
    
    bool isProperInfix(const std::string& infix, const std::string& str) {
        return isInfix(infix, str) && infix != str;
    }
    
    bool isOutfix(const std::string& outfix, const std::string& str) {
        // u is an outfix of v if ∃u1,u2: u = u1u2 ∧ v ∈ u1Σ∗u2
        // This means v can be split to contain u1 as prefix and u2 as suffix
        if (outfix.length() >= str.length()) return false;
        
        for (size_t i = 1; i < outfix.length(); i++) {
            std::string u1 = outfix.substr(0, i);
            std::string u2 = outfix.substr(i);
            if (isPrefix(u1, str) && isSuffix(u2, str)) {
                // Check that the middle part doesn't overlap with u1 and u2
                if (u1.length() + u2.length() <= str.length()) {
                    return true;
                }
            }
        }
        return false;
    }
    
    bool isProperOutfix(const std::string& outfix, const std::string& str) {
        return isOutfix(outfix, str) && outfix != str;
    }
    
    bool overlaps(const std::string& u, const std::string& v) {
        // Check if u and v overlap (u ωol v)
        // Overlap means there exists q such that q is a proper prefix of u and proper suffix of v
        // or vice versa
        if (u.empty() || v.empty()) return false;
        
        // Check proper prefix of u that is proper suffix of v
        for (size_t i = 1; i < u.length(); i++) {
            std::string q = u.substr(0, i);
            if (q.length() < v.length() && isSuffix(q, v)) {
                return true;
            }
        }
        
        // Check proper prefix of v that is proper suffix of u
        for (size_t i = 1; i < v.length(); i++) {
            std::string q = v.substr(0, i);
            if (q.length() < u.length() && isSuffix(q, u)) {
                return true;
            }
        }
        
        return false;
    }
    
    bool isScatteredSubword(const std::string& u, const std::string& v) {
        // Check if u is a scattered subword of v
        if (u.empty()) return true;
        if (u.length() > v.length()) return false;
        
        size_t pos = 0;
        for (char c : u) {
            pos = v.find(c, pos);
            if (pos == std::string::npos) return false;
            pos++;
        }
        return true;
    }
    
    std::string mirror(const std::string& str) {
        std::string result = str;
        std::reverse(result.begin(), result.end());
        return result;
    }
}

// Enum for predicate types
enum class PredicateType {
    PREFIX_FREE,      // Pp
    SUFFIX_FREE,      // Ps
    BIFIX_FREE,       // Pb
    INFIX_FREE,       // Pi
    OUTFIX_FREE,      // Po
    OVERLAP_FREE,     // Pol
    HYPERCODE,        // Ph
    SOLID,            // Psolid
    COMMA_FREE,       // Comma-free (special case of intercode)
    INTERCODE         // General intercode
};

// Predicate class for evaluating code properties
class Predicate {
private:
    PredicateType type;
    int intercodeIndex; // For intercode of index n
    
    // Check intercode property: Σ+C^nΣ+ ∩ C^{n+1} = ∅
    bool checkIntercode(const std::set<std::string>& codeWords) const {
        std::vector<std::string> words(codeWords.begin(), codeWords.end());
        
        // For intercode of index n, check all concatenations of n+1 codewords
        // This implementation focuses on n=1 (comma-free) and n=2 for demonstration
        if (intercodeIndex == 1) {
            // Check all pairs of codewords
            for (size_t i = 0; i < words.size(); i++) {
                for (size_t j = 0; j < words.size(); j++) {
                    std::string concat = words[i] + words[j];
                    
                    // Check if any codeword appears as proper infix
                    for (const std::string& w : words) {
                        if (StringUtils::isProperInfix(w, concat)) {
                            return false;
                        }
                    }
                }
            }
        } else if (intercodeIndex == 2) {
            // Check triples for intercode of index 2
            for (size_t i = 0; i < words.size(); i++) {
                for (size_t j = 0; j < words.size(); j++) {
                    for (size_t k = 0; k < words.size(); k++) {
                        std::string concat = words[i] + words[j] + words[k];
                        
                        for (const std::string& w : words) {
                            if (StringUtils::isProperInfix(w, concat)) {
                                return false;
                            }
                        }
                    }
                }
            }
        }
        
        return true;
    }
    
public:
    Predicate(PredicateType t, int n = 1) : type(t), intercodeIndex(n) {}
    
    bool evaluate(const std::set<std::string>& codeWords) const {
        if (codeWords.empty()) return true;
        
        std::vector<std::string> words(codeWords.begin(), codeWords.end());
        
        // Special handling for intercode
        if (type == PredicateType::INTERCODE) {
            return checkIntercode(codeWords);
        }
        
        // For all other types, check all pairs
        for (size_t i = 0; i < words.size(); i++) {
            for (size_t j = 0; j < words.size(); j++) {
                // Skip i==j for predicates that don't need self-check
                if (i == j) {
                    if (type == PredicateType::OVERLAP_FREE || 
                        type == PredicateType::SOLID) {
                        // Need to check self-overlap
                        if (!evaluatePair(words[i], words[i])) return false;
                    }
                    continue;
                }
                if (!evaluatePair(words[i], words[j])) {
                    return false;
                }
            }
        }
        return true;
    }
    
    bool evaluatePair(const std::string& u, const std::string& v) const {
        switch (type) {
            case PredicateType::PREFIX_FREE:
                return !StringUtils::isProperPrefix(u, v) && 
                       !StringUtils::isProperPrefix(v, u);
                
            case PredicateType::SUFFIX_FREE:
                return !StringUtils::isProperSuffix(u, v) && 
                       !StringUtils::isProperSuffix(v, u);
                
            case PredicateType::BIFIX_FREE:
                // Check both prefix-free AND suffix-free
                return (!StringUtils::isProperPrefix(u, v) && 
                        !StringUtils::isProperPrefix(v, u)) &&
                       (!StringUtils::isProperSuffix(u, v) && 
                        !StringUtils::isProperSuffix(v, u));
                
            case PredicateType::INFIX_FREE:
                return !StringUtils::isProperInfix(u, v) && 
                       !StringUtils::isProperInfix(v, u);
                
            case PredicateType::OUTFIX_FREE:
                return !StringUtils::isProperOutfix(u, v) && 
                       !StringUtils::isProperOutfix(v, u);
                
            case PredicateType::OVERLAP_FREE:
                return !StringUtils::overlaps(u, v);
                
            case PredicateType::HYPERCODE: {
                // Hypercode: no word is a scattered subword of another
                if (u != v && StringUtils::isScatteredSubword(u, v)) return false;
                if (u != v && StringUtils::isScatteredSubword(v, u)) return false;
                return true;
            }
                
            case PredicateType::SOLID:
                // Solid code = infix-free + overlap-free
                return (!StringUtils::isProperInfix(u, v) && 
                        !StringUtils::isProperInfix(v, u)) && 
                       !StringUtils::overlaps(u, v);
                
            case PredicateType::COMMA_FREE:
                // Comma-free: Σ+CΣ+ ∩ C^2 = ∅
                // For any two codewords, neither should be infix of their concatenation
                if (StringUtils::isProperInfix(u, u + v)) return false;
                if (StringUtils::isProperInfix(v, u + v)) return false;
                // Also check the reverse concatenation
                if (StringUtils::isProperInfix(u, v + u)) return false;
                if (StringUtils::isProperInfix(v, v + u)) return false;
                return true;
                
            default:
                return true;
        }
    }
    
    std::string getName() const {
        switch (type) {
            case PredicateType::PREFIX_FREE: return "Prefix-free";
            case PredicateType::SUFFIX_FREE: return "Suffix-free";
            case PredicateType::BIFIX_FREE: return "Bifix-free";
            case PredicateType::INFIX_FREE: return "Infix-free";
            case PredicateType::OUTFIX_FREE: return "Outfix-free";
            case PredicateType::OVERLAP_FREE: return "Overlap-free";
            case PredicateType::HYPERCODE: return "Hypercode";
            case PredicateType::SOLID: return "Solid";
            case PredicateType::COMMA_FREE: return "Comma-free";
            case PredicateType::INTERCODE: 
                return "Intercode (n=" + std::to_string(intercodeIndex) + ")";
            default: return "Unknown";
        }
    }
    
    PredicateType getType() const { return type; }
};

// Represents a decoding (factorization) of a word
class Decoding {
private:
    std::vector<std::string> factors;
    
public:
    Decoding() = default;
    Decoding(const std::vector<std::string>& f) : factors(f) {}
    
    const std::vector<std::string>& getFactors() const { return factors; }
    size_t length() const { return factors.size(); }
    
    std::set<std::string> getCodeWords() const {
        return std::set<std::string>(factors.begin(), factors.end());
    }
    
    std::string toString() const {
        std::string result;
        for (size_t i = 0; i < factors.size(); i++) {
            if (i > 0) result += "·";
            result += factors[i];
        }
        return result;
    }
    
    bool operator==(const Decoding& other) const {
        return factors == other.factors;
    }
};

// Hash for Decoding to use in sets
struct DecodingHash {
    size_t operator()(const Decoding& d) const {
        size_t hash = 0;
        for (const auto& f : d.getFactors()) {
            hash ^= std::hash<std::string>()(f) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

// Main Code class
class Code {
private:
    std::set<std::string> codewords;
    std::string alphabet;
    
    // Cache for decodings
    mutable std::map<std::string, std::vector<Decoding>> decodingCache;
    mutable std::map<std::string, bool> uniquenessCache;
    mutable std::map<std::string, bool> admissibilityCache;
    
    // Find all decodings of a word using DP with backtracking
    std::vector<Decoding> findAllDecodings(const std::string& word) const {
        // Check cache first
        auto it = decodingCache.find(word);
        if (it != decodingCache.end()) {
            return it->second;
        }
        
        // Early return if word is empty
        if (word.empty()) {
            return {Decoding(std::vector<std::string>{})};
        }
        
        std::vector<Decoding> result;
        
        // DP array: each entry contains all decodings of prefix up to i
        std::vector<std::vector<Decoding>> dp(word.length() + 1);
        dp[0].push_back(Decoding(std::vector<std::string>{}));
        
        for (size_t i = 0; i < word.length(); i++) {
            if (dp[i].empty()) continue; // No decodings to this point
            
            for (size_t j = i + 1; j <= word.length(); j++) {
                std::string candidate = word.substr(i, j - i);
                if (codewords.find(candidate) != codewords.end()) {
                    // Found a codeword, extend all decodings up to i
                    for (const Decoding& prefixDecoding : dp[i]) {
                        std::vector<std::string> newFactors = prefixDecoding.getFactors();
                        newFactors.push_back(candidate);
                        dp[j].push_back(Decoding(newFactors));
                    }
                }
            }
        }
        
        result = dp[word.length()];
        
        // Limit cache size to prevent memory issues
        if (decodingCache.size() < 1000) {
            decodingCache[word] = result;
        }
        
        return result;
    }
    
public:
    Code(const std::set<std::string>& words, const std::string& alpha = "abcd") 
        : codewords(words), alphabet(alpha) {
        // Remove empty word if present
        codewords.erase("");
    }
    
    // Get all possible decodings of a word
    std::vector<Decoding> getDecodings(const std::string& word) const {
        return findAllDecodings(word);
    }
    
    // Check if a word is uniquely decodable (optimized)
    bool isUniquelyDecodable(const std::string& word) const {
        auto it = uniquenessCache.find(word);
        if (it != uniquenessCache.end()) {
            return it->second;
        }
        
        // Optimized: find first two decodings without storing all
        std::vector<Decoding> decodings = findAllDecodings(word);
        bool result = decodings.size() == 1;
        uniquenessCache[word] = result;
        return result;
    }
    
    // Verify Proposition 4.3: P-admissibility implies unique decodability
    bool verifyProposition43(const std::string& word, const Predicate& pred) const {
        if (!isPAdmissible(word, pred)) return true; // Not applicable
        return isUniquelyDecodable(word);
    }
    
    // Check if a word is P-admissible
    bool isPAdmissible(const std::string& word, const Predicate& pred) const {
        // Create cache key
        std::string cacheKey = word + "_" + pred.getName();
        auto it = admissibilityCache.find(cacheKey);
        if (it != admissibilityCache.end()) {
            return it->second;
        }
        
        std::vector<Decoding> decodings = findAllDecodings(word);
        if (decodings.empty()) {
            admissibilityCache[cacheKey] = false;
            return false;
        }
        
        // Collect all codewords that appear in any decoding
        std::set<std::string> allCodeWords;
        for (const Decoding& dec : decodings) {
            std::set<std::string> codeWords = dec.getCodeWords();
            allCodeWords.insert(codeWords.begin(), codeWords.end());
        }
        
        // Check every pair of codewords
        std::vector<std::string> words(allCodeWords.begin(), allCodeWords.end());
        for (size_t i = 0; i < words.size(); i++) {
            for (size_t j = 0; j < words.size(); j++) {
                // Skip i==j for predicates that don't need self-check
                if (i == j) {
                    PredicateType type = pred.getType();
                    if (type == PredicateType::OVERLAP_FREE || 
                        type == PredicateType::SOLID) {
                        if (!pred.evaluatePair(words[i], words[i])) {
                            admissibilityCache[cacheKey] = false;
                            return false;
                        }
                    }
                    continue;
                }
                if (!pred.evaluatePair(words[i], words[j])) {
                    admissibilityCache[cacheKey] = false;
                    return false;
                }
            }
        }
        
        admissibilityCache[cacheKey] = true;
        return true;
    }
    
    // Check if this code is a P-code relative to L
    bool isPRelativeCode(const std::set<std::string>& L, const Predicate& pred) const {
        for (const std::string& word : L) {
            if (!isPAdmissible(word, pred)) {
                return false;
            }
        }
        return true;
    }
    
    // Get maximal set of P-admissible words (up to a limit)
    std::set<std::string> getMaxPAdmissibleWords(const Predicate& pred, int maxLength = 8) const {
        std::set<std::string> result;
        
        // Generate all words in C+ up to maxLength using BFS
        std::queue<std::string> toProcess;
        
        // Start with all codewords
        for (const std::string& cw : codewords) {
            if (cw.length() <= maxLength) {
                toProcess.push(cw);
            }
        }
        
        while (!toProcess.empty()) {
            std::string current = toProcess.front();
            toProcess.pop();
            
            std::vector<Decoding> decodings = findAllDecodings(current);
            if (!decodings.empty() && isPAdmissible(current, pred)) {
                result.insert(current);
            }
            
            // Generate concatenations with codewords
            for (const std::string& cw : codewords) {
                std::string next = current + cw;
                if (next.length() <= maxLength) {
                    toProcess.push(next);
                }
            }
        }
        
        return result;
    }
    
    // Extract minimal subset Cq that generates q and is a P-code
    std::set<std::string> getMinimalGeneratingSet(const std::string& q, const Predicate& pred) const {
        std::vector<Decoding> decodings = findAllDecodings(q);
        if (decodings.empty()) return {};
        
        // Find the smallest decoding (by number of unique codewords) that is also a P-code
        std::set<std::string> bestSet;
        size_t bestSize = std::numeric_limits<size_t>::max();
        
        for (const Decoding& dec : decodings) {
            std::set<std::string> candidate = dec.getCodeWords();
            if (pred.evaluate(candidate)) {
                if (candidate.size() < bestSize) {
                    bestSize = candidate.size();
                    bestSet = candidate;
                }
            }
        }
        
        return bestSet;
    }
    
    // Get all minimal generating sets (all possible decodings that are P-codes)
    std::vector<std::set<std::string>> getAllMinimalGeneratingSets(const std::string& q, const Predicate& pred) const {
        std::vector<std::set<std::string>> result;
        std::vector<Decoding> decodings = findAllDecodings(q);
        
        for (const Decoding& dec : decodings) {
            std::set<std::string> candidate = dec.getCodeWords();
            if (pred.evaluate(candidate)) {
                result.push_back(candidate);
            }
        }
        
        return result;
    }
    
    // Check if this is a split code (from the paper)
    bool isSplitCode() const {
        std::set<std::string> current = codewords;
        std::vector<std::set<std::string>> joins;
        
        while (!current.empty()) {
            std::set<std::string> joinSet = getJoin(current);
            if (joinSet.empty()) {
                return current.empty();
            }
            joins.push_back(joinSet);
            
            // Remove joins from current
            for (const std::string& j : joinSet) {
                current.erase(j);
            }
        }
        
        std::cout << "Join decomposition levels: " << joins.size() << "\n";
        return true;
    }
    
    // Get join of a code (words that are joins relative to C*)
    std::set<std::string> getJoin(const std::set<std::string>& code) const {
        std::set<std::string> result;
        
        for (const std::string& w : code) {
            // Check if w is a join relative to C*
            // A word w is a join if for any u, v such that u w v ∈ C*, both u and v are in C*
            bool isJoin = true;
            
            // Check all ways to split w
            for (size_t i = 0; i <= w.length(); i++) {
                std::string prefix = w.substr(0, i);
                std::string suffix = w.substr(i);
                
                // Check if prefix is a concatenation of codewords
                if (!prefix.empty()) {
                    std::vector<Decoding> prefixDec = findAllDecodings(prefix);
                    if (prefixDec.empty()) {
                        isJoin = false;
                        break;
                    }
                }
                
                // Check if suffix is a concatenation of codewords
                if (!suffix.empty()) {
                    std::vector<Decoding> suffixDec = findAllDecodings(suffix);
                    if (suffixDec.empty()) {
                        isJoin = false;
                        break;
                    }
                }
            }
            
            if (isJoin) result.insert(w);
        }
        
        return result;
    }
    
    // Print information about the code
    void printInfo() const {
        std::cout << "Code C = {";
        bool first = true;
        for (const std::string& w : codewords) {
            if (!first) std::cout << ", ";
            std::cout << w;
            first = false;
        }
        std::cout << "}\n";
        
        // Check basic properties
        Predicate prefixPred(PredicateType::PREFIX_FREE);
        Predicate suffixPred(PredicateType::SUFFIX_FREE);
        Predicate bifixPred(PredicateType::BIFIX_FREE);
        Predicate infixPred(PredicateType::INFIX_FREE);
        Predicate outfixPred(PredicateType::OUTFIX_FREE);
        Predicate overlapPred(PredicateType::OVERLAP_FREE);
        Predicate solidPred(PredicateType::SOLID);
        
        std::cout << "Properties:\n";
        std::cout << "  " << std::setw(15) << "Prefix-free: " 
                  << (prefixPred.evaluate(codewords) ? "Yes" : "No") << "\n";
        std::cout << "  " << std::setw(15) << "Suffix-free: " 
                  << (suffixPred.evaluate(codewords) ? "Yes" : "No") << "\n";
        std::cout << "  " << std::setw(15) << "Bifix-free: " 
                  << (bifixPred.evaluate(codewords) ? "Yes" : "No") << "\n";
        std::cout << "  " << std::setw(15) << "Infix-free: " 
                  << (infixPred.evaluate(codewords) ? "Yes" : "No") << "\n";
        std::cout << "  " << std::setw(15) << "Outfix-free: " 
                  << (outfixPred.evaluate(codewords) ? "Yes" : "No") << "\n";
        std::cout << "  " << std::setw(15) << "Overlap-free: " 
                  << (overlapPred.evaluate(codewords) ? "Yes" : "No") << "\n";
        std::cout << "  " << std::setw(15) << "Solid: " 
                  << (solidPred.evaluate(codewords) ? "Yes" : "No") << "\n";
    }
    
    void clearCache() {
        decodingCache.clear();
        uniquenessCache.clear();
        admissibilityCache.clear();
    }
    
    const std::set<std::string>& getCodewords() const { return codewords; }
};

// Demonstration class
class RelativizedCodeSimulator {
private:
    static void printSeparator() {
        std::cout << std::string(60, '=') << "\n";
    }
    
public:
    static void runExample1() {
        printSeparator();
        std::cout << "Example 3.1 from the paper\n";
        std::cout << "C = {a, ab, ba} is not a code\n";
        printSeparator();
        
        std::set<std::string> C = {"a", "ab", "ba"};
        Code code(C, "ab");
        code.printInfo();
        
        // Test specific words
        std::string word1 = "aba";
        std::cout << "\nWord: " << word1 << "\n";
        auto decodings = code.getDecodings(word1);
        std::cout << "Decodings (" << decodings.size() << "):\n";
        for (const Decoding& dec : decodings) {
            std::cout << "  " << dec.toString() << "\n";
        }
        
        Predicate prefixPred(PredicateType::PREFIX_FREE);
        std::cout << "Is P-prefix admissible? " 
                  << (code.isPAdmissible(word1, prefixPred) ? "Yes" : "No") << "\n";
        
        // Verify Proposition 4.3
        std::cout << "Verifies Proposition 4.3? " 
                  << (code.verifyProposition43(word1, prefixPred) ? "Yes" : "No") << "\n";
        
        // Define L = {ab, ba}* ∪ {(ba)^n a^m | n,m ≥ 1}
        std::set<std::string> L = {"ab", "ba", "abba", "baab", "babaa", "bababa"};
        std::cout << "\nChecking if C is P-prefix code relative to L:\n";
        std::cout << "L = {";
        bool first = true;
        for (const std::string& w : L) {
            if (!first) std::cout << ", ";
            std::cout << w;
            first = false;
        }
        std::cout << "}\n";
        std::cout << "Result: " << (code.isPRelativeCode(L, prefixPred) ? "Yes" : "No") << "\n";
        
        // Check P-suffix admissibility
        Predicate suffixPred(PredicateType::SUFFIX_FREE);
        std::cout << "C is P-suffix code relative to L: " 
                  << (code.isPRelativeCode(L, suffixPred) ? "Yes" : "No") << "\n";
        
        std::set<std::string> L2 = {"aab", "abab", "aaa"};
        std::cout << "C is P-suffix code relative to L2: " 
                  << (code.isPRelativeCode(L2, suffixPred) ? "Yes" : "No") << "\n";
    }
    
    static void runExample2() {
        printSeparator();
        std::cout << "Example 3.5 from the paper\n";
        std::cout << "C = {ab, c, ba} relative to L = ({abc} ∪ {cba})*\n";
        printSeparator();
        
        std::set<std::string> C = {"ab", "c", "ba"};
        Code code(C, "abc");
        code.printInfo();
        
        std::set<std::string> L = {"abc", "cba", "abccba", "cbaabc", "abccbaabc"};
        
        std::cout << "\nL = {";
        bool first = true;
        for (const std::string& w : L) {
            if (!first) std::cout << ", ";
            std::cout << w;
            first = false;
        }
        std::cout << "}\n";
        
        Predicate solidPred(PredicateType::SOLID);
        std::cout << "\nIs C a solid code relative to L? " 
                  << (code.isPRelativeCode(L, solidPred) ? "Yes" : "No") << "\n";
        
        // Check specific word
        std::string word = "abccba";
        std::cout << "\nWord: " << word << "\n";
        auto decodings = code.getDecodings(word);
        std::cout << "Decodings (" << decodings.size() << "):\n";
        for (const Decoding& dec : decodings) {
            std::cout << "  " << dec.toString() << "\n";
        }
        std::cout << "Is solid admissible? " 
                  << (code.isPAdmissible(word, solidPred) ? "Yes" : "No") << "\n";
    }
    
    static void runExample3() {
        printSeparator();
        std::cout << "Example 3.6 from the paper\n";
        std::cout << "C = {a, ba, bcabc} is a split code\n";
        printSeparator();
        
        std::set<std::string> C = {"a", "ba", "bcabc"};
        Code code(C, "abc");
        code.printInfo();
        
        std::cout << "\nIs C a split code? " << (code.isSplitCode() ? "Yes" : "No") << "\n";
        
        // Test join extraction
        auto joins = code.getJoin(C);
        std::cout << "Joins in C: {";
        bool first = true;
        for (const std::string& j : joins) {
            if (!first) std::cout << ", ";
            std::cout << j;
            first = false;
        }
        std::cout << "}\n";
        
        // Test with the word from the paper
        std::string word = "abcabcabc";
        std::cout << "\nTesting word: " << word << "\n";
        auto decodings = code.getDecodings(word);
        std::cout << "Decodings:\n";
        for (const Decoding& dec : decodings) {
            std::cout << "  " << dec.toString() << "\n";
        }
    }
    
    static void runHierarchyExample() {
        printSeparator();
        std::cout << "Code Hierarchy Demonstration\n";
        printSeparator();
        
        // Create various code classes
        std::set<std::string> prefixCode = {"ab", "ba", "ca"};
        std::set<std::string> suffixCode = {"ab", "bc", "ac"};
        std::set<std::string> infixCode = {"ab", "cd", "ef"};
        std::set<std::string> outfixCode = {"abc", "def", "ghi"};
        
        Code code1(prefixCode);
        Code code2(suffixCode);
        Code code3(infixCode);
        Code code4(outfixCode);
        
        std::cout << "\nPrefix code example:\n";
        code1.printInfo();
        
        std::cout << "\nSuffix code example:\n";
        code2.printInfo();
        
        std::cout << "\nInfix code example:\n";
        code3.printInfo();
        
        std::cout << "\nOutfix code example:\n";
        code4.printInfo();
        
        std::cout << "\nHierarchy test (L = C+):\n";
        Predicate prefixPred(PredicateType::PREFIX_FREE);
        Predicate suffixPred(PredicateType::SUFFIX_FREE);
        Predicate infixPred(PredicateType::INFIX_FREE);
        
        std::cout << "  C1 is prefix code? " 
                  << (code1.isPRelativeCode(prefixCode, prefixPred) ? "Yes" : "No") << "\n";
        std::cout << "  C2 is suffix code? " 
                  << (code2.isPRelativeCode(suffixCode, suffixPred) ? "Yes" : "No") << "\n";
        std::cout << "  C3 is infix code? " 
                  << (code3.isPRelativeCode(infixCode, infixPred) ? "Yes" : "No") << "\n";
    }
    
    static void runMinimalGeneratingSetExample() {
        printSeparator();
        std::cout << "Minimal Generating Set (Proposition 4.5)\n";
        printSeparator();
        
        std::set<std::string> C = {"a", "ab", "ba"};
        Code code(C);
        
        std::string q = "babaa";
        std::cout << "Word q = " << q << "\n";
        std::cout << "Decodings of q:\n";
        auto decodings = code.getDecodings(q);
        for (const Decoding& dec : decodings) {
            std::cout << "  " << dec.toString() << "\n";
        }
        
        Predicate prefixPred(PredicateType::PREFIX_FREE);
        std::cout << "\nIs q P-prefix admissible? " 
                  << (code.isPAdmissible(q, prefixPred) ? "Yes" : "No") << "\n";
        
        auto Cq = code.getMinimalGeneratingSet(q, prefixPred);
        std::cout << "Minimal generating set Cq = {";
        bool first = true;
        for (const std::string& w : Cq) {
            if (!first) std::cout << ", ";
            std::cout << w;
            first = false;
        }
        std::cout << "}\n";
        
        Code subcode(Cq);
        std::cout << "Is Cq a prefix code? " 
                  << (prefixPred.evaluate(Cq) ? "Yes" : "No") << "\n";
        std::cout << "q in Cq+? " << (!subcode.getDecodings(q).empty() ? "Yes" : "No") << "\n";
        
        // Show all minimal generating sets
        auto allSets = code.getAllMinimalGeneratingSets(q, prefixPred);
        std::cout << "\nAll minimal generating sets (" << allSets.size() << "):\n";
        for (const auto& set : allSets) {
            std::cout << "  {";
            bool first2 = true;
            for (const std::string& w : set) {
                if (!first2) std::cout << ", ";
                std::cout << w;
                first2 = false;
            }
            std::cout << "}\n";
        }
    }
    
    static void runPredicateComparison() {
        printSeparator();
        std::cout << "Predicate Comparison Test\n";
        printSeparator();
        
        // Test various code sets with different predicates
        struct TestCase {
            std::set<std::string> code;
            std::string description;
        };
        
        std::vector<TestCase> testCases = {
            {{"ab", "ba"}, "Prefix code (should be prefix-free)"},
            {{"ab", "bc"}, "Suffix code (should be suffix-free)"},
            {{"abc", "bcd", "cde"}, "Infix code"},
            {{"abc", "def"}, "Outfix code"},
            {{"ab", "ba", "aba"}, "Has overlaps"},
            {{"a", "aa"}, "Has prefixes"},
            {{"ab", "ba", "abba"}, "Has infixes"}
        };
        
        std::vector<PredicateType> predicates = {
            PredicateType::PREFIX_FREE, PredicateType::SUFFIX_FREE, 
            PredicateType::BIFIX_FREE, PredicateType::INFIX_FREE, 
            PredicateType::OUTFIX_FREE, PredicateType::OVERLAP_FREE, 
            PredicateType::SOLID, PredicateType::COMMA_FREE
        };
        
        for (const auto& test : testCases) {
            std::cout << "\nTesting: " << test.description << "\n";
            Code code(test.code);
            code.printInfo();
            
            std::cout << "\nPredicate evaluation results:\n";
            for (PredicateType pt : predicates) {
                Predicate pred(pt);
                bool result = pred.evaluate(test.code);
                std::cout << "  " << std::setw(15) << pred.getName() << ": " 
                          << (result ? "✓" : "✗") << "\n";
            }
        }
    }
    
    static void runOverlapTest() {
        printSeparator();
        std::cout << "Overlap Detection Test\n";
        printSeparator();
        
        std::vector<std::pair<std::string, std::string>> testPairs = {
            {"ab", "bc"},      // Should overlap? "b" is common
            {"abc", "bcd"},    // Should overlap? "bc" is common
            {"ab", "cd"},      // Should not overlap
            {"aba", "bab"},    // Should overlap? "ab" and "ba"
            {"abc", "abc"}     // Self-overlap? Depends on definition
        };
        
        for (const auto& p : testPairs) {
            bool overlaps = StringUtils::overlaps(p.first, p.second);
            std::cout << "overlaps(\"" << p.first << "\", \"" << p.second << "\") = " 
                      << (overlaps ? "true" : "false") << "\n";
        }
    }
    
    static void runComprehensiveTest() {
        printSeparator();
        std::cout << "Comprehensive Test\n";
        printSeparator();
        
        // Create a code and test all predicates
        std::set<std::string> testCode = {"ab", "bc", "cd", "de"};
        Code code(testCode);
        
        std::cout << "Testing code: {ab, bc, cd, de}\n\n";
        
        std::vector<PredicateType> predicates = {
            PredicateType::PREFIX_FREE, PredicateType::SUFFIX_FREE, 
            PredicateType::BIFIX_FREE, PredicateType::INFIX_FREE,
            PredicateType::OUTFIX_FREE, PredicateType::OVERLAP_FREE, 
            PredicateType::SOLID, PredicateType::COMMA_FREE,
            PredicateType::HYPERCODE
        };
        
        for (PredicateType pt : predicates) {
            Predicate pred(pt);
            std::cout << std::setw(20) << pred.getName() << ": " 
                      << (pred.evaluate(testCode) ? "Valid code" : "Not a valid code") << "\n";
        }
        
        // Test some words
        std::vector<std::string> testWords = {"ab", "abbc", "bcde", "abcd", "abcbcd", "bcdeab"};
        Predicate prefixPred(PredicateType::PREFIX_FREE);
        Predicate solidPred(PredicateType::SOLID);
        
        std::cout << "\nP-admissibility test (prefix-free):\n";
        for (const std::string& word : testWords) {
            auto decodings = code.getDecodings(word);
            if (!decodings.empty()) {
                std::cout << "\n" << word << ":\n";
                bool admissible = code.isPAdmissible(word, prefixPred);
                std::cout << "  Prefix-free admissible: " << (admissible ? "Yes" : "No") << "\n";
                bool solidAdmissible = code.isPAdmissible(word, solidPred);
                std::cout << "  Solid admissible: " << (solidAdmissible ? "Yes" : "No") << "\n";
                std::cout << "  Decodings (" << decodings.size() << "):\n";
                for (const Decoding& dec : decodings) {
                    std::cout << "    " << dec.toString() << "\n";
                }
            } else {
                std::cout << "\n" << word << ": Not decodable\n";
            }
        }
    }
    
    static void runPerformanceTest() {
        printSeparator();
        std::cout << "Performance Test\n";
        printSeparator();
        
        std::set<std::string> C = {"a", "ab", "ba", "aba", "bab"};
        Code code(C);
        
        std::vector<std::string> testWords = {
            "aba", "ababa", "babab", "abababa", "bababab"
        };
        
        Predicate prefixPred(PredicateType::PREFIX_FREE);
        
        for (const std::string& word : testWords) {
            auto start = std::chrono::high_resolution_clock::now();
            
            auto decodings = code.getDecodings(word);
            bool admissible = code.isPAdmissible(word, prefixPred);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << "Word: " << word << "\n";
            std::cout << "  Decodings: " << decodings.size() << "\n";
            std::cout << "  Admissible: " << (admissible ? "Yes" : "No") << "\n";
            std::cout << "  Time: " << duration.count() << " μs\n";
        }
    }
    
    static void runAll() {
        runExample1();
        runExample2();
        runExample3();
        runHierarchyExample();
        runMinimalGeneratingSetExample();
        runPredicateComparison();
        runOverlapTest();
        runComprehensiveTest();
        runPerformanceTest();
        
        printSeparator();
        std::cout << "Simulation Complete\n";
        printSeparator();
    }
};

int main() {
    std::cout << "========================================\n";
    std::cout << "Relativized Codes Simulation\n";
    std::cout << "Based on the paper by Daley, Jurgensen, et al.\n";
    std::cout << "Theoretical Computer Science 429 (2012) 54-64\n";
    std::cout << "========================================\n\n";
    
    try {
        RelativizedCodeSimulator::runAll();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}