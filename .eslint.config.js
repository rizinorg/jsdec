module.exports = {
    "languageOptions": {
        "parserOptions": {
            "ecmaVersion": "latest",
            "sourceType": "module",
            "ecmaFeatures": {}
        },
        "globals": {
            "process": true,
            "BigInt": true,
            "Global": true,
            "Limits": true,
            "rizin": true,
            "atob": true,
            "btoa": true,
            "unit": true,
            "console": true
        }
    },
    "rules": {
        "semi": [2, "always"],
        "no-console": ["error", {
            "allow": ["log"]
        }],
        "no-redeclare": ["error", {
            "builtinGlobals": false
        }],
        "no-empty": ["error", {
            "allowEmptyCatch": true
        }],
        "no-unused-vars": ["error", {
            "varsIgnorePattern": "jsdec_|\\binclude\\b",
            "caughtErrors": "none",
            "args": "none"
        }],
        "curly": "error",
        "no-sparse-arrays": "warn",
        "no-cond-assign": ["error", "except-parens"],
        "no-constant-condition": ["error", {
            "checkLoops": false
        }],
        "no-control-regex": "warn"
    }
};