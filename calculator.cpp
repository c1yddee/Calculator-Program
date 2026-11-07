#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWidget>

#include <cctype>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr double PI = 3.14159265358979323846;

// Recursive-descent parser. Grammar (lowest to highest precedence):
//   expr    := term   (('+' | '-') term)*
//   term    := unary  (('*' | '/' | implicit-multiply) unary)*
//   unary   := ('+' | '-') unary | power
//   power   := postfix ('^' unary)?          (right-associative)
//   postfix := primary ('!' | '%')*
//   primary := number | constant | func '(' expr ')' | '(' expr ')'
class Parser {
public:
    Parser(std::string text, bool degrees, double ans)
        : s_(std::move(text)), degrees_(degrees), ans_(ans) {}

    double parse() {
        const double v = expr();
        if (peek() != '\0') fail("unexpected character");
        return v;
    }

private:
    std::string s_;
    size_t pos_ = 0;
    bool degrees_;
    double ans_;

    static const std::set<std::string> &functions() {
        static const std::set<std::string> f = {
            "sin", "cos", "tan", "asin", "acos", "atan",
            "ln", "log", "exp", "sqrt", "abs"};
        return f;
    }

    [[noreturn]] void fail(const std::string &msg) {
        throw std::runtime_error(msg);
    }

    char peek() {
        while (pos_ < s_.size() &&
               std::isspace(static_cast<unsigned char>(s_[pos_])))
            ++pos_;
        return pos_ < s_.size() ? s_[pos_] : '\0';
    }

    bool eat(char c) {
        if (peek() == c) { ++pos_; return true; }
        return false;
    }

    bool startsPrimary() {
        const char c = peek();
        return c == '(' || c == '.' ||
               std::isdigit(static_cast<unsigned char>(c)) ||
               std::isalpha(static_cast<unsigned char>(c));
    }

    double expr() {
        double v = term();
        for (;;) {
            if (eat('+'))      v += term();
            else if (eat('-')) v -= term();
            else               return v;
        }
    }

    double term() {
        double v = unary();
        for (;;) {
            if (eat('*')) {
                v *= unary();
            } else if (eat('/')) {
                const double d = unary();
                if (d == 0.0) fail("division by zero");
                v /= d;
            } else if (startsPrimary()) {
                v *= unary();              // implicit multiply: 2pi, 3(4+5)
            } else {
                return v;
            }
        }
    }

    double unary() {
        if (eat('-')) return -unary();
        if (eat('+')) return unary();
        return power();
    }

    double power() {
        const double base = postfix();
        if (eat('^')) return std::pow(base, unary());
        return base;
    }

    double postfix() {
        double v = primary();
        for (;;) {
            if (eat('!'))      v = factorial(v);
            else if (eat('%')) v /= 100.0;
            else               return v;
        }
    }

    double primary() {
        const char c = peek();
        if (c == '(') {
            ++pos_;
            const double v = expr();
            if (!eat(')')) fail("missing ')'");
            return v;
        }
        if (c == '.' || std::isdigit(static_cast<unsigned char>(c)))
            return number();
        if (std::isalpha(static_cast<unsigned char>(c)))
            return name();
        fail(c == '\0' ? "unexpected end of input" : "unexpected character");
    }

    double number() {
        const size_t start = pos_;
        while (pos_ < s_.size() &&
               (std::isdigit(static_cast<unsigned char>(s_[pos_])) ||
                s_[pos_] == '.'))
            ++pos_;
        return std::stod(s_.substr(start, pos_ - start));
    }

    double name() {
        std::string id;
        while (pos_ < s_.size() &&
               std::isalpha(static_cast<unsigned char>(s_[pos_]))) {
            id += static_cast<char>(std::tolower(
                static_cast<unsigned char>(s_[pos_])));
            ++pos_;
        }

        if (functions().count(id)) {
            if (!eat('(')) fail("expected '(' after " + id);
            const double arg = expr();
            if (!eat(')')) fail("missing ')'");
            return apply(id, arg);
        }
        if (id == "pi")  return PI;
        if (id == "e")   return std::exp(1.0);
        if (id == "ans") return ans_;
        fail("unknown name '" + id + "'");
    }

    double apply(const std::string &f, double x) {
        auto toRad   = [this](double v) { return degrees_ ? v * PI / 180.0 : v; };
        auto fromRad = [this](double v) { return degrees_ ? v * 180.0 / PI : v; };

        if (f == "sin")  return std::sin(toRad(x));
        if (f == "cos")  return std::cos(toRad(x));
        if (f == "tan")  return std::tan(toRad(x));
        if (f == "asin") return fromRad(std::asin(x));
        if (f == "acos") return fromRad(std::acos(x));
        if (f == "atan") return fromRad(std::atan(x));
        if (f == "ln")   return std::log(x);
        if (f == "log")  return std::log10(x);
        if (f == "exp")  return std::exp(x);
        if (f == "sqrt") return std::sqrt(x);
        if (f == "abs")  return std::fabs(x);
        fail("unknown function");
    }

    double factorial(double v) {
        if (v < 0 || v != std::floor(v) || v > 170)
            fail("factorial needs an integer from 0 to 170");
        double r = 1.0;
        for (int i = 2; i <= static_cast<int>(v); ++i) r *= i;
        return r;
    }
};

} // namespace

class Calculator : public QWidget {
public:
    Calculator() {
        setStyleSheet(R"(
            QWidget { background: #2b2b2b; }
            QLineEdit {
                background: #1e1e1e; color: white; border: none;
                border-radius: 6px; padding: 10px; font-size: 24px;
            }
            QPushButton {
                background: #3d3d3d; color: white; border: none;
                border-radius: 6px; padding: 8px; font-size: 15px;
            }
            QPushButton:hover   { background: #4a4a4a; }
            QPushButton:pressed { background: #5a5a5a; }
            QPushButton:checked { background: #6b4fbb; }
        )");

        history_ = new QLabel;
        history_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        setHistoryStyle(false);

        input_ = new QLineEdit;
        input_->setAlignment(Qt::AlignRight);
        input_->setPlaceholderText("0");
        input_->setValidator(new QRegularExpressionValidator(
            QRegularExpression(R"([0-9A-Za-z.+\-*/^%!(), ]*)"), input_));

        connect(input_, &QLineEdit::returnPressed, this, &Calculator::evaluate);
        new QShortcut(QKeySequence(Qt::Key_Equal), this, [this] { evaluate(); });
        new QShortcut(QKeySequence(Qt::Key_Escape), this, [this] { clearAll(); });

        // Each row must add up to 5 columns. "@" codes are special actions.
        struct Key { const char *label; const char *insert; int span; };
        const std::vector<std::vector<Key>> rows = {
            {{"DEG", "@deg", 1}, {"sin", "sin(", 1}, {"cos", "cos(", 1},
             {"tan", "tan(", 1}, {"^", "^", 1}},
            {{"asin", "asin(", 1}, {"acos", "acos(", 1}, {"atan", "atan(", 1},
             {"√", "sqrt(", 1}, {"x²", "^2", 1}},
            {{"ln", "ln(", 1}, {"log", "log(", 1}, {"eˣ", "exp(", 1},
             {"|x|", "abs(", 1}, {"!", "!", 1}},
            {{"(", "(", 1}, {")", ")", 1}, {"π", "pi", 1},
             {"e", "e", 1}, {"%", "%", 1}},
            {{"7", "7", 1}, {"8", "8", 1}, {"9", "9", 1},
             {"/", "/", 1}, {"AC", "@clear", 1}},
            {{"4", "4", 1}, {"5", "5", 1}, {"6", "6", 1},
             {"*", "*", 1}, {"DEL", "@back", 1}},
            {{"1", "1", 1}, {"2", "2", 1}, {"3", "3", 1},
             {"-", "-", 1}, {"Ans", "ans", 1}},
            {{"0", "0", 2}, {".", ".", 1}, {"+", "+", 1}, {"=", "@eval", 1}},
        };

        auto *grid = new QGridLayout;
        grid->setSpacing(6);
        for (int r = 0; r < static_cast<int>(rows.size()); ++r) {
            int c = 0;
            for (const Key &k : rows[r]) {
                auto *btn = new QPushButton(QString::fromUtf8(k.label));
                btn->setFocusPolicy(Qt::NoFocus);   // keep keyboard focus in the input
                btn->setMinimumHeight(44);
                if (QString(k.insert) == "@deg") {
                    btn->setCheckable(true);
                    btn->setChecked(true);
                    deg_ = btn;
                }
                grid->addWidget(btn, r, c, 1, k.span);
                const QString code = QString::fromUtf8(k.insert);
                connect(btn, &QPushButton::clicked, this,
                        [this, code] { onKey(code); });
                c += k.span;
            }
        }

        auto *main = new QVBoxLayout(this);
        main->addWidget(history_);
        main->addWidget(input_);
        main->addLayout(grid);
        setMinimumWidth(360);
        input_->setFocus();
    }

private:
    QLabel *history_ = nullptr;
    QLineEdit *input_ = nullptr;
    QPushButton *deg_ = nullptr;
    bool degrees_ = true;
    double ans_ = 0.0;

    void setHistoryStyle(bool error) {
        history_->setStyleSheet(error
            ? "color: #ff6b6b; font-size: 14px;"
            : "color: #9a9a9a; font-size: 14px;");
    }

    void onKey(const QString &code) {
        if (code == "@clear")     clearAll();
        else if (code == "@back") input_->backspace();
        else if (code == "@eval") evaluate();
        else if (code == "@deg") {
            degrees_ = deg_->isChecked();
            deg_->setText(degrees_ ? "DEG" : "RAD");
        } else {
            input_->insert(code);
        }
        input_->setFocus();
    }

    void evaluate() {
        const QString text = input_->text().trimmed();
        if (text.isEmpty()) return;

        try {
            Parser parser(text.toStdString(), degrees_, ans_);
            const double value = parser.parse();
            if (!std::isfinite(value))
                throw std::runtime_error("result is undefined or too large");

            ans_ = value;
            setHistoryStyle(false);
            history_->setText(text + " =");
            input_->setText(QString::number(value, 'g', 12));
        } catch (const std::exception &e) {
            // Keep the expression so the user can fix it.
            setHistoryStyle(true);
            history_->setText("Error: " + QString::fromUtf8(e.what()));
        }
        input_->setFocus();
    }

    void clearAll() {
        input_->clear();
        history_->clear();
        setHistoryStyle(false);
        input_->setFocus();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Calculator w;
    w.setWindowTitle("Scientific Calculator");
    w.show();
    return app.exec();
}