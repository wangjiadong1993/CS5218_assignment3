//
// Created by wangjiadong on 14/4/18.
//

int main() {
    int a,b,x,y,N, z = 0 ;
    // assume N is an input value
    int i = 0;
    while (i < N) {
        x = -((x + 2*y * 3*z) % 3);
        y = (3*x + 2*y + z) % 11;
        z++;
    }
}

