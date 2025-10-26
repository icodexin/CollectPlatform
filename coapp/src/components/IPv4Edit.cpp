#include "IPv4Edit.h"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStyleOption>
#include <QValidator>

IPv4Edit::IPv4Edit(QWidget* parent)
    : BaseWidget(parent) {
    initUI();
    initConnection();
}

void IPv4Edit::clear() const {
    ui_lineEdit0->clear();
    ui_lineEdit1->clear();
    ui_lineEdit2->clear();
    ui_lineEdit3->clear();
    ui_lineEdit0->setFocus();
}

QString IPv4Edit::address() const {
    return m_address;
}

bool IPv4Edit::eventFilter(QObject* watched, QEvent* event) {
    const auto* edit = qobject_cast<QLineEdit*>(watched);
    if (!edit) return BaseWidget::eventFilter(watched, event);

    const int index = edit->property("index").toInt();
    const int curPos = edit->cursorPosition();
    const int textLen = static_cast<int>(edit->text().length());
    const int selectLen = static_cast<int>(edit->selectedText().length());

    // 键盘事件
    if (event->type() == QEvent::KeyPress) {
        const auto* keyEvent = static_cast<QKeyEvent*>(event);
        const int key = keyEvent->key();

        // 按下小数点, 且不是最后一个输入框, 且光标在最后
        if (key == Qt::Key_Period && index < 3 && curPos == textLen) {
            // 文本不为空时, 切换到下一个输入框
            if (textLen) focusNextChild();
            return true;
        }

        // 按下退格键, 且不是第一个输入框, 且光标在最前, 且没有选中文本
        if (key == Qt::Key_Backspace && index > 0 && curPos == 0 && selectLen == 0) {
            // 切换到上一个输入框
            if (focusPreviousChild()) {
                // 删除上一个输入框的最后一个字符
                if (auto* prevEdit = qobject_cast<QLineEdit*>(focusWidget())) {
                    prevEdit->setCursorPosition(prevEdit->text().length());
                    prevEdit->backspace();
                }
            }
            return true;
        }

        // 按下删除键, 且不是最后一个输入框, 且光标在最后, 且没有选中文本
        if (key == Qt::Key_Delete && index < 3 && curPos == textLen && selectLen == 0) {
            // 切换到下一个输入框
            if (focusNextChild()) {
                // 删除下一个输入框的第一个字符
                if (auto* nextEdit = qobject_cast<QLineEdit*>(focusWidget())) {
                    nextEdit->setCursorPosition(0);
                    nextEdit->del();
                }
            }
            return true;
        }

        // 按下左方向键, 且不是第一个输入框, 且光标在最前
        if (key == Qt::Key_Left && index > 0 && curPos == 0) {
            // 切换到上一个输入框
            if (focusPreviousChild()) {
                // 设置光标在上一个输入框的最后
                if (auto* prevEdit = qobject_cast<QLineEdit*>(focusWidget())) {
                    prevEdit->setCursorPosition(prevEdit->text().length());
                }
            }
            return true;
        }

        // 按下右方向键, 且不是最后一个输入框, 且光标在最后
        if (key == Qt::Key_Right && index < 3 && curPos == textLen) {
            // 切换到下一个输入框
            if (focusNextChild()) {
                // 设置光标在下一个输入框的最前
                if (auto* nextEdit = qobject_cast<QLineEdit*>(focusWidget())) {
                    nextEdit->setCursorPosition(0);
                }
            }
            return true;
        }

        // 按下Home键
        if (key == Qt::Key_Home) {
            ui_lineEdit0->setFocus();
            ui_lineEdit0->setCursorPosition(0);
            return true;
        }

        // 按下End键
        if (key == Qt::Key_End) {
            ui_lineEdit3->setFocus();
            ui_lineEdit3->setCursorPosition(ui_lineEdit3->text().length());
            return true;
        }

        // 匹配到粘贴键
        if (keyEvent->matches(QKeySequence::StandardKey::Paste)) {
            const QString text = QApplication::clipboard()->text();
            setAddress(text);
            return true;
        }
    }
    // 输入法事件
    else if (event->type() == QEvent::InputMethod) {
        const auto* inputEvent = static_cast<QInputMethodEvent*>(event);
        const QString text = inputEvent->commitString();
        // 输入法输入中文时, 包含句号字符, 且不是最后一个输入框, 且光标在最后
        if (text.contains("。") && index < 3 && curPos == textLen) {
            // 文本不为空时, 切换到下一个输入框
            if (textLen) focusNextChild();
            return true;
        }
    }
    // 获得焦点事件
    else if (event->type() == QEvent::FocusIn) {
        setProperty("child-focus", true);
        style()->unpolish(this);
        style()->polish(this);
    }
    // 失去焦点事件
    else if (event->type() == QEvent::FocusOut) {
        setProperty("child-focus", false);
        style()->unpolish(this);
        style()->polish(this);
    }

    return BaseWidget::eventFilter(watched, event);
}

void IPv4Edit::setAddress(const QString& address) {
    static QRegularExpression regex(R"(^(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}$)");
    if (!regex.match(address).hasMatch()) return;

    if (m_address != address) {
        QStringList parts = address.split('.');
        closeConnection();
        ui_lineEdit0->setText(parts[0]);
        ui_lineEdit1->setText(parts[1]);
        ui_lineEdit2->setText(parts[2]);
        ui_lineEdit3->setText(parts[3]);
        ui_lineEdit3->setFocus();
        ui_lineEdit3->setCursorPosition(ui_lineEdit3->text().length());
        m_address = address;
        emit addressChanged(m_address);
        initConnection();
    }
}

void IPv4Edit::onSubTextChanged(const QString& text) {
    if (text.length() == 3 && sender()->property("index").toInt() < 3) {
        focusNextChild();
    }

    QStringList parts = {ui_lineEdit0->text(), ui_lineEdit1->text(), ui_lineEdit2->text(), ui_lineEdit3->text()};
    QString address;
    if (std::all_of(parts.begin(), parts.end(), [](const QString& s) { return !s.isEmpty(); })) {
        address = parts.join(".");
    }
    if (m_address != address) {
        m_address = address;
        emit addressChanged(m_address);
    }
}

void IPv4Edit::initUI() {
    const auto validator = new QRegularExpressionValidator(
        QRegularExpression(R"(^(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)$)"), this);

    const int minWidth = fontMetrics().horizontalAdvance("255") + 2;

    auto createLineEdit = [&](const int index) {
        auto* edit = new QLineEdit(this);
        edit->setObjectName("sub-ip");
        edit->setAlignment(Qt::AlignCenter);
        edit->setMinimumWidth(minWidth);
        edit->setMaximumWidth(minWidth + 5);
        edit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        edit->setValidator(validator);
        edit->setContextMenuPolicy(Qt::NoContextMenu);
        edit->installEventFilter(this);
        edit->setProperty("index", index);
        return edit;
    };

    auto createDotLabel = [this] {
        auto* dot = new QLabel(".", this);
        dot->setAlignment(Qt::AlignCenter);
        dot->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        return dot;
    };

    ui_lineEdit0 = createLineEdit(0);
    ui_lineEdit1 = createLineEdit(1);
    ui_lineEdit2 = createLineEdit(2);
    ui_lineEdit3 = createLineEdit(3);

    auto* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->addStretch(1);
    layout->addWidget(ui_lineEdit0);
    layout->addWidget(createDotLabel());
    layout->addWidget(ui_lineEdit1);
    layout->addWidget(createDotLabel());
    layout->addWidget(ui_lineEdit2);
    layout->addWidget(createDotLabel());
    layout->addWidget(ui_lineEdit3);
    layout->addStretch(1);
}

void IPv4Edit::initConnection() {
    connect(ui_lineEdit0, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
    connect(ui_lineEdit1, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
    connect(ui_lineEdit2, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
    connect(ui_lineEdit3, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
}

void IPv4Edit::closeConnection() {
    disconnect(ui_lineEdit0, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
    disconnect(ui_lineEdit1, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
    disconnect(ui_lineEdit2, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
    disconnect(ui_lineEdit3, &QLineEdit::textChanged, this, &IPv4Edit::onSubTextChanged);
}
