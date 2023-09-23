import os
from pathlib import Path
from time import time
import torch


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


class ZeroTensor:
    def __init__(self, name: str, t: torch.Tensor):
        self.name = name
        assert len(name) < ZERO_MAX_TENSOR_NAME_LEN
        self.ndim = t.ndim
        self.shape = list(t.shape)
        if t.dtype not in torch_dtype_to_zero_dtype:
            raise ValueError(f"Unsupported dtype {t.dtype}")
        self.dtype = torch_dtype_to_zero_dtype[t.dtype]
        self.data = t.numpy().tobytes()
        if t.dtype == torch.bfloat16:
            self.data = bytes([a for i, a in enumerate(self.data) if (i // 2) % 2 == 1])

    def write(self, f):
        pass


with open(f"{model_name}.zero", "wb") as f:
    for bin_filename in bin_filenames:
        t0 = time()
        state_dict = torch.load(bin_filename, map_location=torch.device('cpu'))
        t1 = time()
        print(f"Loaded {bin_filename} in {t1 - t0:.2f} seconds")
        for k, v in state_dict.items():
            print(f"NAME: {k}\tSHAPE: {v.shape}")
            is_bfloat16 = v.dtype == torch.bfloat16
            if is_bfloat16:
                print(f"Converting {k} from bfloat16 to float32")
                v = v.float()
            b = v.numpy().tobytes()
            if is_bfloat16:
                b = bytes([a for i, a in enumerate(b) if (i // 2) % 2 == 1])



