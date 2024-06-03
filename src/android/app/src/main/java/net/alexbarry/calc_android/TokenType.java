package net.alexbarry.calc_android;

enum TokenType {
    // NOTE that these are serialized by using their name,
    // so don't rename them without adding a proper mapping function
    DIGIT,
    VAR,
    FUNC_CALL,
    PAREN_OPEN,
    PAREN_CLOSE,
    OP,
    OTHER,
}
