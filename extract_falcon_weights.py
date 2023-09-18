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
for bin_filename in bin_filenames:
    t0 = time()
    state_dict = torch.load(bin_filename, map_location=torch.device('cpu'))
    t1 = time()
    print(f"Loaded {bin_filename} in {t1 - t0:.2f} seconds")
    for k, v in state_dict.items():
        print(f"NAME: {k}\tSHAPE: {v.shape}")



