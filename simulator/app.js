// app.js - UI interaction for Relativized Codes Simulator (FIXED)

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
        const wordsArray = parseInput(inputStr);
        currentCode = new window.RelativizedLogic.Code(wordsArray);
        
        // Update display
        codeDisplay.textContent = `C = { ${Array.from(currentCode.codewords).join(', ')} }`;
        
        // Evaluate all predicates
        propertiesResults.innerHTML = '';
        
        const predicateTypes = [
            window.RelativizedLogic.PredicateType.PREFIX_FREE,
            window.RelativizedLogic.PredicateType.SUFFIX_FREE,
            window.RelativizedLogic.PredicateType.BIFIX_FREE,
            window.RelativizedLogic.PredicateType.INFIX_FREE,
            window.RelativizedLogic.PredicateType.OUTFIX_FREE,
            window.RelativizedLogic.PredicateType.OVERLAP_FREE,
            window.RelativizedLogic.PredicateType.SOLID,
            window.RelativizedLogic.PredicateType.COMMA_FREE,
            window.RelativizedLogic.PredicateType.HYPERCODE
        ];
        
        predicateTypes.forEach((ptype, index) => {
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
        if (!currentCode) {
            wordTestOutput.textContent = "Please analyze a code first (click 'Analyze Properties').";
            return;
        }
        
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
            const isUnique = currentCode.isUniquelyDecodableFast(word);
            
            outHtml += `<div style="margin-bottom: 1.5rem;">`;
            outHtml += `<strong>Testing word:</strong> <code style="color: #58a6ff;">${word}</code><br>`;
            outHtml += `<strong>Predicate:</strong> ${selectedPredicateType}<br>`;
            outHtml += `<strong>P-Admissible?</strong> <span style="color: ${isAdmissible ? '#3fb950' : '#f85149'}; font-weight: bold;">${isAdmissible ? 'Yes ✓' : 'No ✗'}</span><br>`;
            outHtml += `<strong>Uniquely decodable?</strong> <span style="color: ${isUnique ? '#3fb950' : '#f85149'};">${isUnique ? 'Yes' : 'No'}</span><br><br>`;
            
            outHtml += `<strong>Number of Decodings found:</strong> ${decodings.length}<br>`;
            
            if (decodings.length > 0) {
                outHtml += `<strong>Decodings:</strong><br>`;
                decodings.forEach((dec, idx) => {
                    outHtml += `  ${idx + 1}. ${dec.join(' · ')}<br>`;
                });
            } else {
                outHtml += `  (No decodings over C found)<br>`;
            }
            outHtml += `</div><hr style="border-color: #2d3748;">`;
        });
        
        // Display
        wordTestOutput.innerHTML = outHtml || "No results to display.";
        wordTestOutput.classList.remove('animated');
        void wordTestOutput.offsetWidth;
        wordTestOutput.classList.add('animated');
    }
});