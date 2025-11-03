# Example - c++ - Reed Solomon error correction

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

