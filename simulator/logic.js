// logic.js - Core logic for Relativized Codes (FIXED VERSION)

const StringUtils = {
    isPrefix: (prefix, str) => str.startsWith(prefix),
    isProperPrefix: (prefix, str) => str.startsWith(prefix) && prefix.length < str.length,
    isSuffix: (suffix, str) => str.endsWith(suffix),
    isProperSuffix: (suffix, str) => str.endsWith(suffix) && suffix.length < str.length,
    isInfix: (infix, str) => str.includes(infix),
    isProperInfix: (infix, str) => str.includes(infix) && infix !== str,
    
    isOutfix: (outfix, str) => {
        if (outfix.length >= str.length) return false;
        for (let i = 1; i < outfix.length; i++) {
            let u1 = outfix.substring(0, i);
            let u2 = outfix.substring(i);
            if (str.startsWith(u1) && str.endsWith(u2)) {
                if (u1.length + u2.length <= str.length) return true;
            }
        }
        return false;
    },
    isProperOutfix: (outfix, str) => StringUtils.isOutfix(outfix, str) && outfix !== str,
    
    overlaps: (u, v) => {
        if (!u || !v) return false;
        // Check proper prefix of u that is proper suffix of v
        for (let i = 1; i < u.length; i++) {
            let q = u.substring(0, i);
            if (q.length < v.length && v.endsWith(q)) return true;
        }
        // Check proper prefix of v that is proper suffix of u
        for (let i = 1; i < v.length; i++) {
            let q = v.substring(0, i);
            if (q.length < u.length && u.endsWith(q)) return true;
        }
        return false;
    },
    
    isScatteredSubword: (u, v) => {
        if (!u) return true;
        if (u.length > v.length) return false;
        let pos = 0;
        for (let char of u) {
            pos = v.indexOf(char, pos);
            if (pos === -1) return false;
            pos++;
        }
        return true;
    }
};

const PredicateType = {
    PREFIX_FREE: 'Prefix-free',
    SUFFIX_FREE: 'Suffix-free',
    BIFIX_FREE: 'Bifix-free',
    INFIX_FREE: 'Infix-free',
    OUTFIX_FREE: 'Outfix-free',
    OVERLAP_FREE: 'Overlap-free',
    SOLID: 'Solid',
    COMMA_FREE: 'Comma-free',
    HYPERCODE: 'Hypercode'
};

class Predicate {
    constructor(type) {
        this.type = type;
    }
    
    evaluatePair(u, v) {
        switch (this.type) {
            case PredicateType.PREFIX_FREE:
                return !StringUtils.isProperPrefix(u, v) && !StringUtils.isProperPrefix(v, u);
            case PredicateType.SUFFIX_FREE:
                return !StringUtils.isProperSuffix(u, v) && !StringUtils.isProperSuffix(v, u);
            case PredicateType.BIFIX_FREE:
                return (!StringUtils.isProperPrefix(u, v) && !StringUtils.isProperPrefix(v, u)) &&
                       (!StringUtils.isProperSuffix(u, v) && !StringUtils.isProperSuffix(v, u));
            case PredicateType.INFIX_FREE:
                return !StringUtils.isProperInfix(u, v) && !StringUtils.isProperInfix(v, u);
            case PredicateType.OUTFIX_FREE:
                return !StringUtils.isProperOutfix(u, v) && !StringUtils.isProperOutfix(v, u);
            case PredicateType.OVERLAP_FREE:
                return !StringUtils.overlaps(u, v);
            case PredicateType.HYPERCODE:
                if (u !== v && StringUtils.isScatteredSubword(u, v)) return false;
                if (u !== v && StringUtils.isScatteredSubword(v, u)) return false;
                return true;
            case PredicateType.SOLID:
                return (!StringUtils.isProperInfix(u, v) && !StringUtils.isProperInfix(v, u)) &&
                       !StringUtils.overlaps(u, v);
            case PredicateType.COMMA_FREE:
                if (StringUtils.isProperInfix(u, u + v)) return false;
                if (StringUtils.isProperInfix(v, u + v)) return false;
                if (StringUtils.isProperInfix(u, v + u)) return false;
                if (StringUtils.isProperInfix(v, v + u)) return false;
                return true;
            default:
                return true;
        }
    }
    
    evaluate(codeWords) {
        if (codeWords.size === 0) return true;
        const words = Array.from(codeWords);
        
        // Check all pairs
        for (let i = 0; i < words.length; i++) {
            for (let j = 0; j < words.length; j++) {
                if (i === j) {
                    // For overlap-free and solid, need to check self
                    if (this.type === PredicateType.OVERLAP_FREE || 
                        this.type === PredicateType.SOLID) {
                        if (!this.evaluatePair(words[i], words[i])) return false;
                    }
                    continue;
                }
                if (!this.evaluatePair(words[i], words[j])) return false;
            }
        }
        return true;
    }
    
    getName() {
        return this.type;
    }
}

class Code {
    constructor(wordsArray) {
        this.codewords = new Set(wordsArray.filter(w => w !== ""));
        this.decodingCache = new Map();
        this.uniquenessCache = new Map();
        this.admissibilityCache = new Map();
    }
    
    findAllDecodings(word) {
        // Check cache
        if (this.decodingCache.has(word)) return this.decodingCache.get(word);
        
        if (word === "") return [[]];
        
        // DP array
        let dp = Array.from({length: word.length + 1}, () => []);
        dp[0].push([]); // Empty decoding at start
        
        for (let i = 0; i < word.length; i++) {
            if (dp[i].length === 0) continue;
            for (let j = i + 1; j <= word.length; j++) {
                let candidate = word.substring(i, j);
                if (this.codewords.has(candidate)) {
                    for (let prefixDecoding of dp[i]) {
                        dp[j].push([...prefixDecoding, candidate]);
                    }
                }
            }
        }
        
        let result = dp[word.length];
        // Limit cache size
        if (this.decodingCache.size < 1000) {
            this.decodingCache.set(word, result);
        }
        return result;
    }
    
    getDecodings(word) { 
        return this.findAllDecodings(word); 
    }
    
    isUniquelyDecodable(word) {
        if (this.uniquenessCache.has(word)) return this.uniquenessCache.get(word);
        let decodings = this.findAllDecodings(word);
        let result = decodings.length === 1;
        this.uniquenessCache.set(word, result);
        return result;
    }
    
    isUniquelyDecodableFast(word) {
        if (word === "") return true;
        
        let dp = new Array(word.length + 1).fill(0);
        dp[0] = 1;
        
        for (let i = 0; i < word.length; i++) {
            if (dp[i] === 0) continue;
            for (let j = i + 1; j <= word.length; j++) {
                let candidate = word.substring(i, j);
                if (this.codewords.has(candidate)) {
                    dp[j] += dp[i];
                    if (j === word.length && dp[j] > 1) {
                        return false;
                    }
                }
            }
        }
        return dp[word.length] === 1;
    }
    
    verifyProposition43(word, pred) {
        if (!this.isPAdmissible(word, pred)) return true;
        return this.isUniquelyDecodableFast(word);
    }
    
    isPAdmissible(word, pred) {
        let cacheKey = word + "_" + pred.getName();
        if (this.admissibilityCache.has(cacheKey)) {
            return this.admissibilityCache.get(cacheKey);
        }
        
        let decodings = this.findAllDecodings(word);
        if (decodings.length === 0) {
            this.admissibilityCache.set(cacheKey, false);
            return false;
        }
        
        // Collect all codewords that appear in any decoding
        let allCodeWords = new Set();
        for (let dec of decodings) {
            for (let factor of dec) {
                allCodeWords.add(factor);
            }
        }
        
        // Check every pair of codewords
        let words = Array.from(allCodeWords);
        for (let i = 0; i < words.length; i++) {
            for (let j = 0; j < words.length; j++) {
                if (i === j) {
                    // For overlap-free and solid, need to check self
                    if (pred.type === PredicateType.OVERLAP_FREE || 
                        pred.type === PredicateType.SOLID) {
                        if (!pred.evaluatePair(words[i], words[i])) {
                            this.admissibilityCache.set(cacheKey, false);
                            return false;
                        }
                    }
                    continue;
                }
                if (!pred.evaluatePair(words[i], words[j])) {
                    this.admissibilityCache.set(cacheKey, false);
                    return false;
                }
            }
        }
        
        this.admissibilityCache.set(cacheKey, true);
        return true;
    }
    
    isPRelativeCode(L, pred) {
        for (let word of L) {
            if (!this.isPAdmissible(word, pred)) {
                return false;
            }
        }
        return true;
    }
    
    getMinimalGeneratingSet(q, pred) {
        let decodings = this.findAllDecodings(q);
        if (decodings.length === 0) return new Set();
        
        let bestSet = null;
        let bestSize = Infinity;
        
        for (let dec of decodings) {
            let candidate = new Set(dec);
            if (pred.evaluate(candidate)) {
                if (candidate.size < bestSize) {
                    bestSize = candidate.size;
                    bestSet = candidate;
                }
            }
        }
        
        return bestSet || new Set();
    }
    
    getAllMinimalGeneratingSets(q, pred) {
        let result = [];
        let decodings = this.findAllDecodings(q);
        
        for (let dec of decodings) {
            let candidate = new Set(dec);
            if (pred.evaluate(candidate)) {
                result.push(candidate);
            }
        }
        
        return result;
    }
    
    printInfo() {
        console.log("Code C = {", Array.from(this.codewords).join(", "), "}");
        
        let predicates = [
            PredicateType.PREFIX_FREE,
            PredicateType.SUFFIX_FREE,
            PredicateType.BIFIX_FREE,
            PredicateType.INFIX_FREE,
            PredicateType.OUTFIX_FREE,
            PredicateType.OVERLAP_FREE,
            PredicateType.SOLID
        ];
        
        for (let ptype of predicates) {
            let pred = new Predicate(ptype);
            console.log(`  ${ptype}: ${pred.evaluate(this.codewords) ? "Yes" : "No"}`);
        }
    }
}

// Export for app.js
window.RelativizedLogic = {
    Code,
    Predicate,
    PredicateType,
    StringUtils
};