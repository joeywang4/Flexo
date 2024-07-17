"""Test all circuits are generated correctly"""

import os
from hashlib import sha256

file_path = os.path.dirname(os.path.abspath(__file__))
base_path = os.path.join(file_path, "../../")
warnings = 0


def check_files(path: str, hashes: dict, ext=".elf"):
    """Ensure all 50 files exist for a circuit, and there is no hash collision"""
    global warnings

    for i in range(50):
        filename = os.path.join(base_path, path + f"-{i+1}" + ext)
        filename = os.path.normpath(filename)
        if not os.path.exists(filename):
            print(f"[!] Installation incomplete. File {filename} does not exist!")
            exit()

        with open(filename, "rb") as infile:
            data = infile.read()
        file_hash = sha256(data).hexdigest()
        if file_hash in hashes:
            print(
                f"[!] Warning: hash collision found for {filename} and {hashes[file_hash]}"
            )
            warnings += 1
        else:
            hashes[file_hash] = filename


if __name__ == "__main__":
    hashes = {}

    # circuits
    check_files("reproduce/build/ALU/ALU", hashes)
    check_files("reproduce/build/sha1_round/sha1_round", hashes)
    check_files("reproduce/build/aes_round/aes_round", hashes)
    check_files("reproduce/build/simon25/simon25", hashes)
    check_files("reproduce/build/simon32/simon32", hashes)
    check_files("reproduce/build/sha1_2blocks/sha1_2blocks", hashes)
    check_files("reproduce/build/aes_block/aes_block", hashes)

    # packer circuits
    check_files("UPFlexo/WM/build/simon25_CTR/simon25_CTR", hashes, ".S")
    check_files("UPFlexo/WM/build/simon32_CTR/simon32_CTR", hashes, ".S")
    check_files("UPFlexo/WM/build/AES_CTR/AES_CTR", hashes, ".S")

    # packers
    check_files("UPFlexo/build/simon25_CTR/upx-simon25_CTR", hashes)
    check_files("UPFlexo/build/simon32_CTR/upx-simon32_CTR", hashes)
    check_files("UPFlexo/build/AES_CTR/upx-AES_CTR", hashes)

    # packed executables
    check_files("reproduce/build/ls-simon25_CTR/ls-simon25_CTR", hashes)
    check_files("reproduce/build/ls-simon32_CTR/ls-simon32_CTR", hashes)
    check_files("reproduce/build/ls-AES_CTR/ls-AES_CTR", hashes)

    if warnings == 0:
        print("[*] Test passed. Installed successfully.")
    else:
        print(f"[*] Test passed with {warnings} warnings.")
