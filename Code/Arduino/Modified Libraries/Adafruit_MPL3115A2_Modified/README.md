## Modifications

A small change on line 62 of the cpp file was made to change the oversampling ratio from 128 to 1. This speeds up the pressure samples from ~400ms to ~10ms each.

### Before
```cpp
  _ctrl_reg1.reg = MPL3115A2_CTRL_REG1_OS128 | MPL3115A2_CTRL_REG1_ALT;
```

### After
```cpp
  _ctrl_reg1.reg = MPL3115A2_CTRL_REG1_OS1 | MPL3115A2_CTRL_REG1_ALT;
```