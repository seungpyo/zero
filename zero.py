import ctypes
import torch

ZERO_MAX_TENSOR_NAME_LEN = 128
torch_dtype_to_zero_dtype = {
    torch.float32: 0,
    torch.int32: 1,
    torch.int64: 2,
    torch.bfloat16: 3,
}

zero = ctypes.CDLL('./zero_disk_object.so') 
libc = ctypes.CDLL('libSystem.dylib')
# libc = ctypes.CDLL('libc.so.6') # On Windows, use 'msvcrt.dll' instead
class ZeroDiskObject(ctypes.Structure):
    _fields_ = [
        ("hash", ctypes.c_uint32),
        ("offset", ctypes.c_uint64),
        ("size", ctypes.c_uint64),
        ("data", ctypes.c_void_p),
    ]

class ZeroTensor(ctypes.Structure):
    _fields_ = [
        ("name", ctypes.c_char * ZERO_MAX_TENSOR_NAME_LEN),
        ("dtype", ctypes.c_uint32),
        ("ndim", ctypes.c_uint32),
        ("shape", ctypes.POINTER(ctypes.c_uint64)),
        ("data", ctypes.c_void_p),
    ]
    @staticmethod
    def from_tensor(t: torch.Tensor, name: str = None):
        zt = ZeroTensor()
        zt.name = name.encode('utf-8') if name is not None else b''
        zt.dtype = torch_dtype_to_zero_dtype[t.dtype]
        zt.ndim = t.ndim
        zt.shape = (ctypes.c_uint64 * len(t.shape))(*t.shape)
        zt.data = t.data_ptr()
        return zt


if __name__ == "__main__":
    tensors = [torch.tensor([i, i + 1, i + 2]) for i in range(10)]
    with open("capi.zero", "wb") as f:
        fd = f.fileno()
        f.write(len(tensors).to_bytes(4, byteorder='little'))
        f.seek(0)
        offset = 0
        zdos = [ZeroDiskObject() for _ in tensors]
        for i, zdo in enumerate(zdos):
            print(f"Writing header for tensor {i}, offset = {offset}")
            offset = zero.zero_disk_object_write_header_fd(ctypes.byref(zdos[i]), fd, offset)
        for i, tensor in enumerate(tensors):
            zt = ZeroTensor.from_tensor(tensor, f"tensor_{i:02d}")
            zero.zero_disk_object_serialize_tensor(ctypes.byref(zdos[i]), ctypes.byref(zt))
            print("Serialized: zdo.hash=", zdo.hash, "zdo.offset=", zdo.offset, "zdo.size=", zdo.size)
            print(f"Writing data for tensor {i}, offset = {offset}")
            offset = zero.zero_disk_object_write_data_fd(ctypes.byref(zdo), fd, offset)








