#include "parcer.h"

#include <iostream>

namespace s21 {

Parcer& Parcer::operator=(const Parcer& other) {
  expression_ = other.expression_;
  num_stack_ = other.num_stack_;
  op_stack_ = other.op_stack_;
  is_valid_ = other.is_valid_;
  return *this;
};
Parcer& Parcer::operator=(Parcer&& other) noexcept {
  expression_ = std::move(other.expression_);
  num_stack_ = std::move(other.num_stack_);
  op_stack_ = std::move(other.op_stack_);
  is_valid_ = std::move(other.is_valid_);
  return *this;
};

Parcer& Parcer::operator()(const std::string& str, const std::string& x) {
  Clear();
  expression_ = str;
  if (expression_.size() > 255) expression_.resize(255);
  if (ValidatorForX()) {
    Replace(x);
    ReadString();
  }
  return *this;
};

void Parcer::Calculate(const std::string& str, size_t priority) {
  if (str == ")") {
    while (!op_stack_.empty() && op_stack_.top().first != "(")
      ChooseCalculateMode();
    op_stack_.pop();
    if (!op_stack_.empty() && op_stack_.top().second == 4)
      ChooseCalculateMode();
  } else {
    while (!op_stack_.empty() && op_stack_.top().second >= priority)
      ChooseCalculateMode();
  }
};

void Parcer::ChooseCalculateMode() {
  if (num_stack_.size() > 1 && op_stack_.top().second < 4)
    OperationMode();
  else if (num_stack_.size() > 0 && op_stack_.top().second == 4)
    FunctionMode();
};

double Parcer::ReadString() {
  char* ptr{&expression_[0]};
  char* end{&expression_[expression_.size()]};
  Validator();
  if (is_valid_) {
    while (is_valid_ && ptr != end) {
      if (is_valid_) CheckDigit(&ptr, end);
      if (is_valid_) CheckFunction(&ptr, end);
      if (is_valid_) CheckOperator(&ptr, end);
    }
    while (!op_stack_.empty() && !num_stack_.empty()) {
      if (op_stack_.size() == 1 && num_stack_.size() == 1) break;
      ChooseCalculateMode();
    }
    if (!op_stack_.empty() || num_stack_.empty()) is_valid_ = false;
  }
  return num_stack_.empty() ? 0 : num_stack_.top();
};

void Parcer::AddToStack(std::string& buffer) {
  if (!buffer.empty() && is_valid_) {
    size_t new_priority = TakePriority(buffer);
    CheckUnary(new_priority);
    if (buffer != "(" && CheckPow(buffer)) Calculate(buffer, new_priority);
    if (buffer != ")") op_stack_.push({buffer, new_priority});
    buffer.clear();
  }
};

size_t Parcer::TakePriority(const std::string& op) noexcept {
  size_t priority{};
  if (op.find_first_of("()") != (size_t)-1) {
    priority = 0;
  } else if (op.find_first_of("+-") != (size_t)-1) {
    priority = 1;
  } else if (op.find_first_of("*/%") != (size_t)-1) {
    priority = 2;
  } else if (op.find_first_of("^") != (size_t)-1) {
    priority = 3;
  } else {
    priority = 4;
  }
  return priority;
};

void Parcer::OperationMode() {
  double second = num_stack_.top();
  num_stack_.pop();
  double first = num_stack_.top();
  num_stack_.pop();
  std::string op = op_stack_.top().first;
  op_stack_.pop();
  if (op == "+")
    num_stack_.push(first + second);
  else if (op == "-")
    num_stack_.push(first - second);
  else if (op == "*")
    num_stack_.push(first * second);
  else if (op == "/")
    std::fabs(second) > __DBL_EPSILON__ ? num_stack_.push(first / second)
                                        : num_stack_.push(NAN);
  else if (op == "^")
    num_stack_.push(std::pow(first, second));
  else if (op == "%")
    num_stack_.push(std::fmod(first, second));
  else
    is_valid_ = false;
};

void Parcer::FunctionMode() {
  double first = num_stack_.top();
  num_stack_.pop();
  std::string op = op_stack_.top().first;
  op_stack_.pop();
  if (op == "sin")
    num_stack_.push(std::sin(first));
  else if (op == "cos")
    num_stack_.push(std::cos(first));
  else if (op == "tan")
    num_stack_.push(std::tan(first));
  else if (op == "asin")
    num_stack_.push(std::asin(first));
  else if (op == "acos")
    num_stack_.push(std::acos(first));
  else if (op == "atan")
    num_stack_.push(std::atan(first));
  else if (op == "sqrt")
    num_stack_.push(std::sqrt(first));
  else if (op == "ln")
    num_stack_.push(std::log(first));
  else if (op == "log")
    num_stack_.push(std::log10(first));
  else
    is_valid_ = false;
};

void Parcer::Validator() {
  int bracer{};
  char prev{};
  bool has_dot{};
  is_valid_ = true;
  for (auto it : expression_) {
    if (it == '(')
      ++bracer;
    else if (it == ')')
      --bracer;
    if (prev && std::strchr("+-*/%^", prev) && std::strchr("+-*/%^", it))
      is_valid_ = false;
    else if ((!prev || prev == '(') && std::strchr("*/%^", it))
      is_valid_ = false;
    else if (prev && it == ')' &&
             (std::strchr("+-*/%^", prev) ||
              (std::isalpha(prev) && prev != 'x')))
      is_valid_ = false;
    else if (prev == ')' && std::isdigit(it))
      is_valid_ = false;
    if (bracer < 0) is_valid_ = false;
    if (it == '.' && has_dot) is_valid_ = false;
    if (!std::isdigit(it) && it != '.') has_dot = false;
    if (it == '.') has_dot = true;
    prev = it;
  }
  if (std::strchr("+-*/%^(", prev)) is_valid_ = false;
};

bool Parcer::ValidatorForX() {
  char prev{};
  is_valid_ = true;
  for (auto it : expression_) {
    if (prev == 'x' && it == 'x') {
      is_valid_ = false;
      break;
    } else if (((std::isdigit(prev) || prev == '.') && it == 'x') ||
               (prev == 'x' && (std::isdigit(it) || it == '.'))) {
      is_valid_ = false;
      break;
    } else if (prev == 'x' && std::isalpha(it)) {
      is_valid_ = false;
      break;
    }
    prev = it;
  }
  return is_valid_;
}

void Parcer::CheckDigit(char** ptr, char* end) {
  std::string buffer{};
  while (*ptr != end && (std::isdigit(**ptr) || **ptr == '.'))
    buffer += *((*ptr)++);
  if (!buffer.empty()) {
    if (buffer == ".") buffer = "0";
    num_stack_.push(std::stod(buffer));
    digit_last_ = true;
  }
};
void Parcer::CheckFunction(char** ptr, char* end) {
  std::string buffer{};
  while (*ptr != end && std::isalpha(**ptr)) buffer += *((*ptr)++);
  if (!buffer.empty()) {
    if (digit_last_) is_valid_ = false;
    if (CheckCorrectFunction(buffer)) AddToStack(buffer);
    digit_last_ = false;
  }
};

bool Parcer::CheckCorrectFunction(const std::string& str) {
  const std::vector<std::string> standard{"sin",  "cos",  "tan", "asin", "acos",
                                          "atan", "sqrt", "ln",  "log"};
  bool res{};
  for (auto it : standard)
    if (it == str) res = true;
  is_valid_ = res;
  return is_valid_;
};

void Parcer::CheckOperator(char** ptr, char* end) {
  std::string buffer{};
  if (*ptr != end && std::ispunct(**ptr) && **ptr != '.') buffer += *((*ptr)++);
  if (!buffer.empty()) {
    if (CheckCorrectOperator(buffer)) AddToStack(buffer);
    digit_last_ = false;
  }
};

bool Parcer::CheckCorrectOperator(const std::string& str) {
  if (!op_stack_.empty() && op_stack_.top().second == 4 &&
      TakePriority(str) > 0 && !digit_last_)
    is_valid_ = false;
  return is_valid_;
};

bool Parcer::CheckPow(const std::string& buffer) const noexcept {
  return op_stack_.empty() || op_stack_.top().first != "^" || buffer != "^";
};

void Parcer::CheckUnary(const size_t& priority) noexcept {
  if ((num_stack_.empty() ||
       (!op_stack_.empty() && op_stack_.top().first == "(")) &&
      priority == 1 && !digit_last_) {
    num_stack_.push(0);
  }
};

void Parcer::Clear() {
  expression_ = {};
  while (!op_stack_.empty()) op_stack_.pop();
  while (!num_stack_.empty()) num_stack_.pop();
  is_valid_ = false;
  digit_last_ = false;
}

void Parcer::Replace(const std::string& x) {
  for (int i = static_cast<int>(expression_.size()) - 1; i >= 0; --i) {
    if (expression_[i] == 'x') expression_.replace(i, 1, x);
  }
}

std::pair<std::vector<double>, std::vector<double>> Parcer::CreateDots(
    const std::string& str, std::pair<int, int>& x_borders,
    std::pair<int, int>& y_borders) {
  expression_ = str;
  std::string num_in_str;
  std::pair<std::vector<double>, std::vector<double>> result;
  if (ValidatorForX()) {
    if (x_borders.first >= x_borders.second ||
        y_borders.first >= y_borders.second) {
      x_borders.first = -10;
      y_borders.first = -10;
      x_borders.second = 10;
      y_borders.second = 10;
    }
    int top_border = abs(x_borders.first) + abs(x_borders.second);
    result.first.reserve(top_border * 10);
    result.second.reserve(top_border * 10);
    for (int i = 0; i < top_border * 10; ++i) {
      num_in_str = "(" + std::to_string(i / 10.0 + x_borders.first) + ")";
      operator()(str, num_in_str);
      result.first.push_back(i / 10.0 + x_borders.first);
      result.second.push_back(Answer());
    }
  }
  return result;
};

}  // namespace s21
