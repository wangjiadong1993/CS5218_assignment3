//
// Created by wangjiadong on 19/4/18.
//

int main() {
    int x, a;
    a = 10;
    int b = 5;
    if (a > 0)
        x = 3 + b;
    else
        x = 3 - b;
    assert (x >= 0);
    return x;
}