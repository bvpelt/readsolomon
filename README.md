# Example - c++ - Reed Solomon error correction

## Explain
Everything appens in a *finite field* usually GF(2<sup>8</sup>) - the Galois Field of 256 elements (bytes).

In the field:
- each symbol is 1 byte (8 bits)
- Addition = XOR
- Multiplication = polynomal arithmetic mod in an irreducible polynomial (like x<sup>8</sup> + x<sup>4</sup> + x<sup>3</sup> + x<sup>2</sup> + 1)

All operations are reversible and consistent.

You start with:
- k data symbols (8 bits?)
- you add (n-k) parity symbols (bits) which gives a codeword of length n

The code is notated as RS(n,k)

You can correct up to t = (n - k) / 2 symbol errors
or detect up to n - k symbol errors
or recover up to n - k missing symbols

Example:
A Byte is 8 bits k = 8, a Symbol is a bit!!!
Codeword length n = 12
Parity bits = 12-8 = 4

Correct up to t  = (12 - 8)/2 = 2 symbol error
Detect up to 12 - 8 = 4 symbol errors
Recover up to 12 - 8 = 4 missing symbols

### Encoding
Data bytes are coefficients of a polynomal.


> D(x) = d<sub>0</sub> + d<sub>1</sub>x<sup>1</sup> + d<sub>2</sub>x<sup>2</sup> + ... + d<sub>k - 1</sub>x<sup>k - 1</sup>


Then RS (ReedSolomon algorithm) adds parity symbols so the *final codeword* C(x) is divisable by a *generator polynomal* G(x).

> C(x) = D(x) * x<sup>(n - k)</sup> + P(x)

And C(x) is divisible by G(x).

This ensures a unique relation between data and parity.

Encoding is just *polynomial division modulo G(x)*

> P(x) = remainder of D(x) * x<sup>(n - k)</sup> : G(x)

Then append the parity bytes to the message.

### Decoding

A receiver computes the *syndromes* which indicate whether errors exist and where they may be.
Syndromes are evaluations of the received polynomals.

> S_i = R($\alpha$<sup>i</sup>) for i = 1 ... n-k

where $\alpha$ is a generator element in GF(2<sup>8</sup>)

- If all S_i = 0 there is no error
- Otherwise there are errors

The error decoder finds:
- *Error locator polynomial* $\Lambda$(x) which positions are wrong
- *Error evaluator polynomial* $\Omega$(x) how big the errors are

This step uses the *Berlekamp–Massey* algorithm or *Euclidean algorithm*.

Then it solves for the error values via *Forney’s algorithm*.

Once the positions and values of errors are found:
> Corrected[i] = Received[i] $\oplus$ Error[i]




## Compile using

```bash
make all
```
## Pseudocode

```text
# Encode
bytes = utf8_encode(text)
k = 1024
parity_bytes = 64
blocks = split_into_blocks(bytes, k)
for each block in blocks:
    parity = RS_encode(block, k, k+parity_bytes)
    write_block_to_sidecar(block_index, parity)

# Decode
for each block_index:
    block = read_block_from_file_or_zeroes(...)
    parity = read_parity(block_index)
    recovered_block = RS_decode(block + parity)
    if decode_failed:
       report unrecoverable
    else:
       append recovered_block
text = utf8_decode(concatenate(recovered_blocks))
```

## Usage

After compiling
- encode: generate the control file file.parity which is needed for recovery
- decode: use input file and control file file.parity to recover

Using an input file: example.html for encoding and example.html for decoding without error correction and example-add.html with extra characters at the end for error correction the following commands are executed.

### Encode

```bash
./rs_file_encode input.txt # generates file.parity
./reed -e input.txt        # generates input.txt.parity
```

### Decode

#### No error correction needed

```bash
./rs_file_decode input.txt file.parity # generates recovered.txt
diff input.txt recovered.txt
```

#### Error correction needed

```bash
./rs_file_decode input_corrupt.txt file.parity
diff input.txt recovered.txt                # compare the original file, no differences
diff input_corrupt.txt recovered.txt        # compare the input to the corrected file, differences since corrected

./reed -d input.txt
diff input.txt input.txt.recovered          # compare the original file, no differences
diff input_corrupt.txt input.txt.recovered  # compare the input to the corrected file, differences since corrected

# Alternative
make test
```

