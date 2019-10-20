#ifndef UART_H
#define UART_H

namespace lightguy {

class UART {
public:
    static UART &instance();

    void transmit(int ch);

private:
    bool initialized = false;
    void init();
};

}
#endif  // #ifndef UART_H
