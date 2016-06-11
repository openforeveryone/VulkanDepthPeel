
struct Vertex{
    float posX, posY, posZ, posW; // Position data
    float r, g, b, a;             // Color
};

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f
#define XYZp5(_x_, _y_, _z_) (_x_), (_y_), (_z_), 0.6

static const struct Vertex vertexData[] = {
        {XYZ1(-1, -1, -1), XYZp5(0.f, 0.f, 0.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},

        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(1, 1, 1), XYZp5(1.f, 1.f, 1.f)},

        {XYZ1(1, 1, 1), XYZp5(1.f, 1.f, 1.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},

        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, -1, -1), XYZp5(0.f, 0.f, 0.f)},

        {XYZ1(1, 1, 1), XYZp5(1.f, 1.f, 1.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},

        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(-1, -1, -1), XYZp5(0.f, 0.f, 0.f)},
};
