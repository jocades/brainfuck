#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#define DEBUG_PRINT_CODE

enum Kind {
  Inc = '+',
  Dec = '-',
  Left = '<',
  Right = '>',
  Input = ',',
  Output = '.',
  JmpIfZero = '[',
  JmpIfNonZero = ']',
};

struct Op {
  Kind kind;
  size_t operand;

  Op(Kind kind, size_t operand = 0) : kind(kind), operand(operand) {}
};

class Lexer {
 private:
  std::string_view source;
  size_t cursor = 0;

 public:
  Lexer(const std::string& source) : source(source) {}

  bool iscmd(char c) {
    switch (c) {
      case '+':
      case '-':
      case '>':
      case '<':
      case '[':
      case ']':
      case '.':
      case ',': return true;
      default: return false;
    }
  }

  bool iseof() {
    return cursor >= source.length();
  }

  void advance() {
    cursor++;
  }

  void skip() {
    while (!iseof() && !iscmd(source[cursor])) advance();
  }

  char next() {
    skip();
    if (iseof()) return '\0';
    return source[cursor++];
  }
};

std::vector<Op> compile(const std::string& source) {
  Lexer lexer(source);
  std::vector<Op> code;
  std::stack<size_t> stack;

  char c = lexer.next();
  while (c) {
    switch (c) {
      case '+':
      case '-':
      case '>':
      case '<':
      case '.': {
        size_t count = 1;
        char n = lexer.next();
        while (c == n) {
          count++;
          n = lexer.next();
        }
        code.emplace_back((Kind)c, count);
        c = n;
        break;
      }
      case '[': {
        stack.push(code.size());
        code.emplace_back((Kind)c);
        c = lexer.next();
        break;
      };
      case ']': {
        if (stack.empty()) {
          std::cerr << "Error: Loop mismatch\n";
          exit(1);
        };
        size_t index = stack.top();
        stack.pop();
        code.emplace_back((Kind)c, index + 1);
        code[index].operand = code.size();
        c = lexer.next();
        break;
      }
      default: break;
    };
  }
  return code;
}

bool interpret(const std::string& source) {
  std::vector<Op> code = compile(source);

#ifdef DEBUG_PRINT_CODE
  for (size_t i = 0; i < code.size(); i++) {
    printf("%zu: %c (%zu)\n", i, code[i].kind, code[i].operand);
  }
#endif

  std::vector<uint8_t> mem(30000, 0);
  size_t dp = 0, ip = 0;

  while (ip < code.size()) {
    const Op op = code[ip];
    switch (op.kind) {
      case Inc: {
        mem[dp] += op.operand;
        ip++;
        break;
      }
      case Dec: {
        mem[dp] -= op.operand;
        ip++;
        break;
      }
      case Left: {
        if (dp < op.operand) {
          std::cerr << "Runtime Error: Memory underflow\n";
          return false;
        }
        dp -= op.operand;
        ip++;
        break;
      }
      case Right: {
        dp += op.operand;
        if (dp >= mem.size()) {
          std::cerr << "Runtime Error: Memory overflow\n";
          return false;
        }
        ip++;
        break;
      }
      case Input: assert(0 && "Input is not yet implemented"); break;
      case Output: {
        for (size_t i = 0; i < op.operand; i++) std::cout << mem[dp];
        ip++;
        break;
      }
      case JmpIfZero: {
        if (mem[dp] == 0) ip = op.operand;
        else ip++;
        break;
      }
      case JmpIfNonZero: {
        if (mem[dp] != 0) ip = op.operand;
        else ip++;
        break;
      }
      default: assert(0 && "Unreachable"); break;
    }
  }
  return true;
}

std::string read_to_string(const std::string& filename) {
  std::ifstream f(filename);
  if (!f) throw std::runtime_error("failed to open file");
  std::stringstream buf;
  buf << f.rdbuf();
  return buf.str();
}

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: bf [path]\n";
    exit(64);
  }

  std::string source = read_to_string(argv[1]);

  if (!interpret(source)) return 70;
  return 0;
}
