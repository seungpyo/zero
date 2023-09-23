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
    with open("capi.zero", "wb") as f:
        fd = f.fileno()
        zt = ZeroTensor.from_tensor(torch.tensor([1, 2, 3]), "my_weight:123")
        zdo = ZeroDiskObject()
        zero.zero_disk_object_serialize_tensor(ctypes.byref(zdo), ctypes.byref(zt))
        print("Serialized: zdo.hash=", zdo.hash, "zdo.offset=", zdo.offset, "zdo.size=", zdo.size)
        zdo.offset = 100
        offset = zero.zero_disk_object_write_header_fd(ctypes.byref(zdo), fd, 0)
        offset = zero.zero_disk_object_write_data_fd(ctypes.byref(zdo), fd, offset)








