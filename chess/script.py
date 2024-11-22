import re

def extract_function_names(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
        
    # Regular expression to match C function declarations
    # This pattern looks for:
    # - return type (including modifiers)
    # - function name
    # - parameter list in parentheses
    # - opening brace of function body
    pattern = r'\b(?:void|int|char|float|double|long|short|unsigned|signed|size_t|bool)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^)]*\)\s*{'
    
    # Find all matches
    functions = re.finditer(pattern, content)
    
    # Extract and sort function names
    function_names = sorted(match.group(1) for match in functions)
    
    print("Found functions:")
    print("---------------")
    for i, name in enumerate(function_names, 1):
        print(f"{i}. {name}")
    
    print(f"\nTotal functions found: {len(function_names)}")
    return function_names

# Usage
file_path = 'main.c'  # Replace with your file path
functions = extract_function_names(file_path)