# This is the input and output files of the tester- you can also see it in the json
## I highly recommend u to add a lot more cases to the json and to share it with everyone
### 1.
### **`data.txt`** - BEFORE 
```
1234567890abcdefghij

```

### **`requests.txt`**
```
R 2 15
R 0 2
W 100 OMERTHEKING
W 10 HEMI
Q
```
### **`data.txt`** - AFTER
```
1234567890HEMIabcdefghij

```
### **`read_results.txt`** - AFTER
```
34567890abcdef
123

```
### 2.
### **`data.txt`** - BEFORE
```
1234567890abcdefghij

```

### **`requests.txt`**
```
W 10 HelloWorld
R 5 29
W 100 InsertHere
R 90 150
W 110 XYZABC
R 195 220
W 20 LongTextHere
Q
```
### **`data.txt`** - AFTER
```
1234567890HelloWorldLongTextHereabcdefghij

```
### **`read_results.txt`** - AFTER
```
67890HelloWorldabcdefghij

```
### 3.
### **`data.txt`**  - BEFORE
```
1234567890abcdefghij

```

### **`requests.txt`**
```
R 2 2
W 9 NEVERGONNAGIVEYOUUP
R 0 25
W 0 98
W 10 WINDOWSISTRASH
R 2 30
Q
```
### **`data.txt`** - AFTER
```
9812345678WINDOWSISTRASH9NEVERGONNAGIVEYOUUP0abcdefghij

```
### **`read_results.txt`** - AFTER
```
3
123456789NEVERGONNAGIVEYOU
12345678WINDOWSISTRASH9NEVERG

```
