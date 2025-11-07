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

#### Error correction
The encoder generates the codeword

> C(x) = D(x) * x<sup>(n - k)</sup> + P(x)

In the input for the decode you receive input (R) which potentially might be wrong (E)

> R(x) = C(x) + E(x)

Then syndrome calculation uses

> S_i = R($\alpha$<sup>i</sup>) for i = 1 ... 2t

Where t = (n-k)/2 and $\alpha$ = a primitive element of the Galois Field.

To find the error locator polynomal one uses

> $\Lambda$(x) = 1 + $\Lambda$<sub>1</sub>x + $\Lambda$<sub>2</sub>x<sup>2</sup> + ... + $\Lambda$<sub>t</sub>x<sup>t</sup>

The roots correspond to the reciprocals of the error locations

> x = $\alpha$<sup>(-i<sub>1</sub>)</sup>, $\alpha$<sup>(-i<sub>2</sub>)</sup>, ...

The key equasion to solve is

> $\Lambda$(x) * S(x) $\equiv$ $\Omega$(x) (mod x<sup>(2t)</sup>)

- S(x) is the syndrome polynomial (this is known/given)
- $\Lambda$(x) is the error locator polynomial (unknown)
- $\Omega$(x) is the evaluator polynomial (unknown)

This can be done using;
- Berlekamp–Massey Algorithm
- Extended Euclidean Algorithm

Finding the error locations
Once we have Λ(x), the error positions correspond to the inverses of its roots:

> $\Lambda$(x) = 0 $\implies$ x = $\alpha$<sup>-i</sup>
You find these roots by Chien search — brute-forcing over all possible powers of α.

Now we know where the errors are, but we also need how large they are — the actual value to subtract from the corrupted symbol.

For each error position j (at location x<sub>j</sub> = $\alpha$<sup>(−i<sub>j</sub>)</sup>):

Forney’s formula gives:

E<sub>j</sub>=$\Omega$(x<sub>j</sub>)<sup>-1</sup> / $\Lambda$'(x<sub>j</sub>)<sup>-1</sup>

where
- $\Omega$(x) is the error evaluator polynomial (from before)
- $\Lambda$'(x) is derivate of $\Lambda$(x)
- The division and evaluation are all in GF(2<sup>8</sup>)

Once E<sub>j</sub> is known the recovery is

R[i<sub>j</sub>] = R[i<sub>j</sub>] - E<sub>j</sub>


#### Method: Berlekamp–Massey Algorithm

Berlekamp–Massey iteratively finds the shortest linear feedback shift register (LFSR) that generates the observed syndrome sequence.

High-level idea:
- Start with $\Lambda$(x) = 1.
- Iterate through syndromes S₁, S₂, ….
- Compute the discrepancy d at each step.
- If d $\neq$ 0, update $\Lambda$(x) by combining it with a previous version scaled by d.

It’s elegant because:
- It’s efficient (O(t<sup>2</sup>)).
- Works well when implemented in hardware or software.
- Produces $\Lambda$(x) and, optionally, $\Omega$(x) as a byproduct

#### Method: Extended Euclidean Algorithm

Alternatively, we can solve the key equation directly using polynomial division in GF(2<sup>8</sup>):

> $\Lambda$(x) * S(x) $\equiv$ $\Omega$(x) (mod x<sup>(2t)</sup>)

This is done using the Extended Euclidean Algorithm, similar to how you compute GCDs of integers — but over polynomials.

We compute the GCD of x<sup>(2t)</sup> and S(x) while tracking Bézout coefficients to find $\Lambda$(x) and $\Omega$(x) that satisfy the congruence.

Why use Euclidean instead of Berlekamp–Massey?
- It’s conceptually simpler for mathematicians (no iterative “discrepancy” updates).
- Works well for small code lengths.
- Berlekamp–Massey is typically faster and better for hardware.

# Code example
A concrete Reed–Solomon example.
We’ll use a very small RS(7, 3) code over GF(2<sup>3</sup>) (to keep arithmetic small enough to follow).
That means:
- n = 7 symbols total
- k = 3 data symbols
- n–k = 4 parity symbols
- Can correct up to t = 2 symbol errors

## Step 0 - Setup
We work in GF(2³) defined by the primitive polynomial

p(x) = x<sup>3</sup> + x + 1

The primitive element $\alpha$ satisfies $\alpha$<sup>3</sup> = $\alpha$ + 1

The field elements (in polynomial and binary form):

| Power                | Polynomial           | Binary | Decimal |
|----------------------|----------------------|--------|---------|
| $\alpha$<sup>0</sup> |                    1 |    001 |       1 |
| $\alpha$<sup>1</sup> | $\alpha$<sup>1</sup> |    010 |       2 |
| $\alpha$<sup>2</sup> | $\alpha$<sup>2</sup> |    100 |       4 |
| $\alpha$<sup>3</sup> | $\alpha$ + 1         |    011 |       3 |
| $\alpha$<sup>4</sup> | $\alpha$<sup>2</sup> +  $\alpha$ |    110 |       6 |
| $\alpha$<sup>5</sup> | $\alpha$<sup>2</sup> +  $\alpha$ + 1 |    111 |       7 |
| $\alpha$<sup>6</sup> | $\alpha$<sup>2</sup> + 1|    101 |       5 |

After $\alpha$<sup>6</sup> the pattern repeats.

## Step 1 — Message and Encoding
In this example we encode M(x) = 4x<sup>2</sup> + 5x + 6 (three data symbols).

The generator polynomial for RS(7, 3) is built from (x - $\alpha$<sup>1</sup>)(x - $\alpha$<sup>2</sup>)(x - $\alpha$<sup>3</sup>)(x - $\alpha$<sup>4</sup>):

g(x) = x<sup>4</sup> + g<sub>1</sub>x<sup>3</sup> + g<sub>2</sub>x<sup>2</sup> + g<sub>3</sub>x + g<sub>4</sub>

(over GF(2<sup>3</sup>), not expanded in full here)

We encode we multiply the message by x<sup>n - k</sup> = x<sup>4</sup> and divide by g(x):

x<sup>4</sup>M(x) = 4x<sup>6</sup> + 5x<sup>5</sup> + 6x<sup>4</sup>

The remainder gives the parity symbols.

Let’s skip the raw division; suppose the resulting codeword is:

C = [4,5,6,2,7,0,3]

## Step 2 — Simulate Transmission and Errors

Say we transmit C, but two symbols get corrupted:

| Index | Sent | Received | Remark |
|-------|------|----------|--------|
|     0 |    4 |        4 |        |
|     1 |    5 |        5 |        |
|     2 |    6 |        2 | Error  |
|     3 |    2 |        7 | Error  |
|     4 |    7 |        7 |        |
|     5 |    0 |        0 |        |
|     6 |    3 |        3 |        |

Received word R = [4,5,2,7,7,0,3]

## Step 3 — Compute Syndromes
We compute S<sub>i</sub> = R($\alpha$<sup>i</sup>) for i=1..4 since 2t = 4.

After plugging in and reducing (details omitted but mechanical), we get:

| i | S<sub>i</sub>        |
|---|----------------------|
| 1 | $\alpha$<sup>4</sup> |
| 2 | $\alpha$<sup>2</sup> |
| 3 | $\alpha$<sup>1</sup> |
| 4 | $\alpha$<sup>6</sup> |

So S(x) = S<sub>1</sub> + S<sub>2</sub>x+ S<sub>3</sub>x<sup>2</sup>+ S<sub>4</sub>x<sup>3</sup> = $\alpha$<sup>4</sup> + $\alpha$<sup>2</sup>x + $\alpha$ x<sup>2</sup> + $\alpha$<sup>6</sup>x<sup>3</sup> 

## Step 4 — Find Error Locator Polynomial $\Lambda$(x)

We need $\Lambda$(x) = 1 + $\Lambda$<sub>1</sub>x + $\Lambda$<sub>2</sub>x<sup>2</sup>

We can use Berlekamp–Massey to solve:

$\Lambda$(x)S(x) $\equiv$ $\Omega$(x) (mod x<sup>4</sup>)

For brevity, after applying the iterative updates:

- $\Lambda$(x) = 1 + $\alpha$<sup>6</sup>x + $\alpha$<sup>5</sup>x<sup>2</sup>
- $\Omega$(x) = $\alpha$<sup>2</sup> + $\alpha$<sup>4</sup>x

## Step 5 — Find Error Locations (Chien Search)
We test x = $\alpha$<sup>-0</sup>...$\alpha$<sup>-6</sup> and find where $\Lambda$(x) = 0.

We find zeros at
- x = $\alpha$<sup>-2</sup>
- x = $\alpha$<sup>-3</sup>

Thus, error locations correspond to symbol positions:
- i<sub>1</sub> = 2
- i<sub>2</sub> = 3

(these match our simulated corruptions!)

## Step 6 — Forney’s Algorithm (Error Magnitudes)
For each root x<sub>j</sub> = $\alpha$<sup>-1<sub>j</sub></sup>:

E<sub>j</sub> = - $\Omega$ (x<sub>j</sub><sup>-1</sup>) / $\Lambda$'(x<sub>j</sub><sup>-1</sup>)

Compute the derivative:
$\Lambda$'(x) = $\alpha$<sup>6</sup> + ( 2 $\alpha$<sup>5</sup>x) -> in GF(2<sup>3</sup>), derivate drops even powers so 
$\Lambda$'(x) = $\alpha$<sup>6</sup>

Evaluate $\Omega$ and $\Lambda$' at those x<sub>j</sub> and compute E<sub>j</sub> (GF Arithmic):

| j | x<sub>j</sub>        | E<sub>j</sub>        |
|---|----------------------|----------------------|
|  1| $\alpha$<sup>5</sup> | $\alpha$<sup>2</sup> |
|  2| $\alpha$<sup>4</sup> | $\alpha$<sup>6</sup> |

## Step 7 — Correct the Errors

Subtract the error magnitudes at positions 2 and 3:

| Index | R | -E                       | Corrected |
|-------|---|--------------------------|-----------|
|      2|  2| $\alpha$<sup>2</sup> = 4 |          6|
|      3|  7| $\alpha$<sup>6</sup> = 5 |          2|

Recovered codeword

C(x) = [4,5,6,2,7,0,3]

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

Special characters see https://www.upyesp.org/posts/makrdown-vscode-math-notation/
