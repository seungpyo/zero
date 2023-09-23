import os
from pathlib import Path
from time import time
import torch
import ctypes
from zero import zero, ZeroDiskObject, ZeroTensor

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
    for bin_filename in bin_filenames:
        state_dict = torch.load(bin_filename, map_location=torch.device('cpu'))
        num_tensors += len(state_dict)

    with open(f"{model_name}.zero", "wb") as f:
        fd = f.fileno()
        f.write(num_tensors.to_bytes(4, byteorder='little'))
        data_offset = (ctypes.sizeof(ZeroDiskObject) - ctypes.sizeof(ctypes.c_void_p)) * num_tensors + ctypes.sizeof(ctypes.c_uint32)
        for bin_filename in bin_filenames:
            state_dict = torch.load(bin_filename, map_location=torch.device('cpu'))
            disk_objects = [ZeroDiskObject() for _ in state_dict]
            for i, (k, v) in enumerate(state_dict.items()):
                print(f"NAME: {k}\tSHAPE: {v.shape}")
                is_bfloat16 = v.dtype == torch.bfloat16
                if is_bfloat16:
                    print(f"Converting {k} from bfloat16 to float32")
                    v = v.float()
                b = v.numpy().tobytes()
                if is_bfloat16:
                    b = bytes([a for i, a in enumerate(b) if (i // 2) % 2 == 1])
                zt = ZeroTensor.from_tensor(v, k)



