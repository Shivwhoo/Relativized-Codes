// app.js - UI interaction for Relativized Codes Simulator

document.addEventListener('DOMContentLoaded', () => {
    const codewordsInput = document.getElementById('codewords-input');
    const analyzeBtn = document.getElementById('analyze-btn');
    const propertiesResults = document.getElementById('properties-results');
    const codeDisplay = document.getElementById('code-display');
    
    const testWordInput = document.getElementById('test-word-input');
    const predicateSelect = document.getElementById('predicate-select');
    const testWordBtn = document.getElementById('test-word-btn');
    const wordTestOutput = document.getElementById('word-test-output');
    
    let currentCode = null;
    let currentWordsArray = [];

    // Initialize with default
    analyzeCode();

    analyzeBtn.addEventListener('click', analyzeCode);
    
    testWordBtn.addEventListener('click', testWord);
    
    function parseInput(inputStr) {
        return inputStr.split(',')
            .map(s => s.trim())
            .filter(s => s.length > 0);
    }
    
    function analyzeCode() {
        const inputStr = codewordsInput.value;
        currentWordsArray = parseInput(inputStr);
        currentCode = new window.RelativizedLogic.Code(currentWordsArray);
        
        // Update display
        codeDisplay.textContent = `C = { ${Array.from(currentCode.codewords).join(', ')} }`;
        
        // Evaluate all predicates
        propertiesResults.innerHTML = '';
        
        const predicates = Object.values(window.RelativizedLogic.PredicateType);
        
        predicates.forEach((ptype, index) => {
            const pred = new window.RelativizedLogic.Predicate(ptype);
            const passed = pred.evaluate(currentCode.codewords);
            
            // Create badge
            const badge = document.createElement('div');
            badge.className = `property-badge animated ${passed ? 'success' : 'fail'}`;
            badge.style.animationDelay = `${index * 0.05}s`;
            
            badge.innerHTML = `
                <div class="icon">${passed ? '✓' : '×'}</div>
                <span>${ptype}</span>
            `;
            
            propertiesResults.appendChild(badge);
        });
    }
    
    function testWord() {
        if (!currentCode) return;
        
        const rawInput = testWordInput.value;
        const wordsToTest = parseInput(rawInput);
        
        if (wordsToTest.length === 0) {
            wordTestOutput.textContent = "Please enter at least one word to test.";
            return;
        }
        
        const selectedPredicateType = predicateSelect.value;
        const pred = new window.RelativizedLogic.Predicate(selectedPredicateType);
        
        let outHtml = "";
        
        wordsToTest.forEach(word => {
            const decodings = currentCode.getDecodings(word);
            const isAdmissible = currentCode.isPAdmissible(word, pred);
            
            outHtml += `Testing word: <strong>${word}</strong>\n`;
            outHtml += `Predicate: <strong>${selectedPredicateType}</strong>\n`;
            outHtml += `P-Admissible? <strong>${isAdmissible ? 'Yes ✅' : 'No ❌'}</strong>\n\n`;
            
            outHtml += `Number of Decodings found: ${decodings.length}\n`;
            
            if (decodings.length > 0) {
                decodings.forEach((dec, idx) => {
                    outHtml += `  ${idx + 1}. ${dec.join(' · ')}\n`;
                });
            } else {
                outHtml += `  (No decodings over C found)\n`;
            }
            outHtml += `\n----------------------------------------\n\n`;
        });
        
        // Display
        wordTestOutput.innerHTML = outHtml;
        wordTestOutput.classList.remove('animated');
        void wordTestOutput.offsetWidth; // trigger reflow
        wordTestOutput.classList.add('animated');
    }
});
