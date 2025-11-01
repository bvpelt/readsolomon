# Example - c++ - Reed Solomon error correction

## Compile using

```bash
g++ -std=c++11 rs_file_encode.cpp -O3 -o rs_file_encode
g++ -std=c++11 rs_file_decode.cpp -O3 -o rs_file_decode
```

## Usage

After compiling
- encode: generate the control file file.parity which is needed for recovery
- decode: use input file and control file file.parity to recover

Using an input file: example.html for encoding and example.html for decoding without error correction and example-add.html with extra characters at the end for error correction the following commands are executed.

### Encode

```bash
./rs_file_encode example.html
```

This generates a file.parity

### Decode

#### No error correction needed

```bash
./rs_file_decode example.html file.parity
diff example.html recovered.txt
```

This generates recovered.txt which is identical to example.html

#### Error correction needed

```bash
./rs_file_decode example-add.html file.parity
diff example.html recovered.txt # compare the original file, no differences
diff example-add.html recovered.txt # compare the input to the corrected file, differences since corrected
```

This generates recovered.txt which is identical to example.html