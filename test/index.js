
const func2 = () => {
    let sum = 0;
    const arr = new Array(1000000);
    for (let i = 0; i < 1000000; i++) {
        arr[i] = i;
        sum += i;
    }
};

const func1 = () => {
    for (let i = 0; i < 1000000; i++) {
        func2();
    }
};
while(1) {
    func1();
}