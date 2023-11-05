// uses Sphere from sphere_struct.glsl

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 3) readonly buffer inputs {
    Sphere spheres_in[];
};

layout(std430, binding = 4) writeonly buffer outputs {
    Sphere spheres_out[];
};

layout(std430, binding = 5) readonly buffer keys {
    uint map[];
};

uniform vec4 gravity;
uniform vec3 low_bound;
uniform vec3 high_bound;
uniform float dt;
uniform float collision_multiplier;
uniform uint object_count;

uint hash(uint x) {
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

uint cell_hash(vec4 position) {
    vec4 cell_pos = floor(position / 1.0);
    uint cell_hash = hash(floatBitsToUint(cell_pos.x));
    cell_hash = hash(cell_hash + floatBitsToUint(cell_pos.y));
    cell_hash = hash(cell_hash + floatBitsToUint(cell_pos.z));
    return cell_hash;
}

void main() {
    // a shader által számolt gömb indexe
    uint i = gl_GlobalInvocationID.x;
    // ha túlindexelnénk a tömböt, akkor ne
    if (i >= object_count) {
        return;
    }
    Sphere s = spheres_in[i];
    bool collided = false;
    int xoff = 0;
    int yoff = 0;
    int zoff = 0;
    // végigmegy a gömb körülötti 3*3*3 cellán
    // HACK: a cellák mérete 1-nek van tekintve
    for (int xoff = -1; xoff <= 1; xoff += 1) {
        for (int yoff = -1; yoff <= 1; yoff += 1) {
            for (int zoff = -1; zoff <= 1; zoff += 1) {
                // kiszámolja a cellához tartozó hash-t és kulcsot
                uint hash = cell_hash(s.center + vec4(xoff, yoff, zoff, 0));
                uint key = hash % 256;
                // végig iterál a kulcshoz tartozó gömbökön
                for (uint index = map[key]; index < map[key+1]; ++index) {
                    // standard buta, de kb jó ütközés szimuláció
                    Sphere other = spheres_in[index];
                    if (other.cell_hash == hash && s.obj_id != other.obj_id) {
                        vec4 dir = s.center - other.center;
                        if (length(dir) < s.radius + other.radius) {
                            dir = normalize(dir);
                            s.velocity = s.velocity - dot(s.velocity - other.velocity, dir)
                                / dot(dir, dir) * dir;
                            vec4 coll_center = vec4(
                                (s.center.xyz * other.radius + other.center.xyz * s.radius)
                                / (s.radius + other.radius), 1);
                            s.center = coll_center + dir * (s.radius);
                            collided = true;
                        }
                    }
                }
            }
        }
    }
    // for (uint j = 0; j < object_count; ++j) {
    //     if (s.obj_id != spheres_in[j].obj_id) {
    //         Sphere other = spheres_in[j];
    //         // if (s.cell_hash == other.cell_hash) {
    //         vec4 dir = s.center - other.center;
    //         if (length(dir) < s.radius + other.radius) {
    //             dir = normalize(dir);
    //             s.velocity = s.velocity - dot(s.velocity - other.velocity, dir)
    //                 / dot(dir, dir) * dir;
    //             vec4 coll_center = vec4(
    //                 (s.center.xyz * other.radius + other.center.xyz * s.radius)
    //                 / (s.radius + other.radius), 1);
    //             s.center = coll_center + dir * (s.radius);
    //             collided = true;
    //         }
    //         // }
    //     }
    // }
    s.velocity += gravity * dt;
    s.center += s.velocity * dt;
    if (s.center.x > high_bound.x - s.radius) {
        s.center.x = high_bound.x - s.radius;
        s.velocity.x = -abs(s.velocity.x);
        collided = true;
    }
    if (s.center.y > high_bound.y - s.radius) {
        s.center.y = high_bound.y - s.radius;
        s.velocity.y = -abs(s.velocity.y);
        collided = true;
    }
    if (s.center.z > high_bound.z - s.radius) {
        s.center.z = high_bound.z - s.radius;
        s.velocity.z = -abs(s.velocity.z);
        collided = true;
    }
    if (s.center.x < low_bound.x + s.radius) {
        s.center.x = low_bound.x + s.radius;
        s.velocity.x = +abs(s.velocity.x);
        collided = true;
    }
    if (s.center.y < low_bound.y + s.radius) {
        s.center.y = low_bound.y + s.radius;
        s.velocity.y = +abs(s.velocity.y);
        collided = true;
    }
    if (s.center.z < low_bound.z + s.radius) {
        s.center.z = low_bound.z + s.radius;
        s.velocity.z = +abs(s.velocity.z);
        collided = true;
    }
    if (collided) {
        s.velocity *= collision_multiplier;
    }
    uint chash = cell_hash(s.center);
    s.cell_hash = chash;
    s.cell_key = chash % 256;
    spheres_out[i] = s;
}
