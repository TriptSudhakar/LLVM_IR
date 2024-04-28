int test1(int x, int y){
    if (x > 1){
        return x;
    }
    else{
        return y;
    }
    
    // this code will be eliminated
    x = x + y;
    x = x - y;
    return 0;
}

int main(){
    test1(5,4);
    return 0;
    // this code will be eliminated
    int x;
}