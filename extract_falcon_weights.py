import os
from pathlib import Path
from time import time
import torch
import ctypes
from zero import zero, ZeroDiskObject, ZeroTensor
from tqdm import tqdm

hf_cache_dir = os.path.join(str(Path.home()), '.cache', 'huggingface', 'hub')
model_name = "models--tiiuae--falcon-7b"
snapshot_dir = os.path.join(hf_cache_dir, model_name, "snapshots")
latest_snapshot_dirname = max(
    os.listdir(snapshot_dir), 
    key=lambda x: int(os.path.getmtime(os.path.join(snapshot_dir, x))),
)
latest_snapshot_dir = os.path.join(snapshot_dir, latest_snapshot_dirname)
bin_filenames = [os.path.join(latest_snapshot_dir, x) for x in os.listdir(latest_snapshot_dir) if x.endswith(".bin")]
bin_filenames.sort()

ZERO_MAX_TENSOR_NAME_LEN = 128
torch_dtype_to_zero_dtype = {
    torch.float32: 0,
    torch.int32: 1,
    torch.int64: 2,
    torch.bfloat16: 3,
}

if __name__ == "__main__":
    num_tensors = 0
    print("Counting tensors...")
    for bin_filename in bin_filenames:
        state_dict = torch.load(bin_filename, map_location=torch.device('cpu'))
        num_tensors += len(state_dict)
    print(f"Found {num_tensors} tensors in {len(bin_filenames)} files")
    pbar = tqdm(total=num_tensors)
    with open(f"{model_name}.zero", "wb") as f:
        fd = f.fileno()
        f.write(num_tensors.to_bytes(4, byteorder='little'))
        f.seek(0)
        for i, bin_filename in enumerate(bin_filenames):
            pbar.set_description(f"Loading file {i + 1}/{len(bin_filenames)}")
            tensors = torch.load(bin_filename, map_location=torch.device('cpu'))
            pbar.set_description(f"Converting file {i + 1}/{len(bin_filenames)}")
            zdos = [ZeroDiskObject() for _ in tensors]
            header_offset = 4
            offset = (ctypes.sizeof(ZeroDiskObject) - ctypes.sizeof(ctypes.c_void_p)) * num_tensors + ctypes.sizeof(ctypes.c_uint32)
            for i, (name, tensor) in enumerate(tensors.items()):
                zt = ZeroTensor.from_tensor(tensor, name)
                zero.zero_disk_object_serialize_tensor(ctypes.byref(zdos[i]), ctypes.byref(zt))
                zdos[i].offset = offset
                offset += zdos[i].size
            for i, zdo in enumerate(zdos):
                header_offset = zero.zero_disk_object_write_header_fd(ctypes.byref(zdos[i]), fd, header_offset)
                zero.zero_disk_object_write_data_fd(ctypes.byref(zdos[i]), fd, zdo.offset)
                pbar.update(1)





