import torch
from torch import onnx
from transformers import AutoTokenizer, AutoModelForCausalLM

model = "tiiuae/falcon-7b"

tokenizer = AutoTokenizer.from_pretrained(model)
print("Loading tokenizer ok")
model = AutoModelForCausalLM.from_pretrained(
    model, 
    trust_remote_code=True,
)
print("Loading model ok")
onnx.export(
    model, 
    torch.zeros((1, 1), dtype=torch.long),
    f"{model.__class__.__name__}.onnx",
    input_names=["input_ids"],
    output_names=["output"],
    opset_version=12,
    do_constant_folding=True,
    dynamic_axes={
        "input_ids": {0: "batch_size", 1: "sequence_length"},
        "output": {0: "batch_size", 1: "sequence_length"},
    },
)

