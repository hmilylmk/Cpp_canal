#include "protobuf.h"

int main(void)
{
    Client c1("127.0.0.1", 1111);
    c1.connect();
    c1.checkValid("canal", "canal");
    return 0;
}
