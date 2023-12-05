layout(std430, binding = 2) readonly buffer input_keys {
    uint k[];
};

layout(std430, binding = 5) writeonly buffer output_keys {
    uint ko[];
};

