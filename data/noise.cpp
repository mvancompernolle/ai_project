#include "noise.h"

/*! \file
    \ingroup Math

    $Id: noise.cpp,v 1.3 2003/05/10 17:00:41 nigels Exp $

    $Log: noise.cpp,v $
    Revision 1.3  2003/05/10 17:00:41  nigels
    Tidy and document

*/

#include <math/round.h>

const Vector GltNoisePerlin::_vector[16] =
{
    Vector( 1, 1, 0),
    Vector(-1, 1, 0),
    Vector( 1,-1, 0),
    Vector(-1,-1, 0),
    Vector( 1, 0, 1),
    Vector(-1, 0, 1),
    Vector( 1, 0,-1),
    Vector(-1, 0,-1),
    Vector( 0, 1, 1),
    Vector( 0,-1, 1),
    Vector( 0, 1,-1),
    Vector( 0,-1,-1),
    Vector( 1, 1, 0),
    Vector(-1, 1, 0),
    Vector( 0,-1, 1),
    Vector( 0,-1,-1)
};

GltNoisePerlin::GltNoisePerlin(const uint32 seed)
:
  _type(NOISE_PERLIN),
  _seed(seed),
  _octaves(1),
  _lambda(1),
  _omega(0.5)
{
}

GltNoisePerlin::GltNoisePerlin(const GltNoisePerlin &noise)
{
    _type    = noise._type;
    _seed    = noise._seed;
    _octaves = noise._octaves;
    _lambda  = noise._lambda;
    _omega   = noise._omega;
}

GltNoisePerlin::~GltNoisePerlin()
{
}

// Get/Set

GltNoisePerlin::NoiseType &GltNoisePerlin::type()    { return _type;    }
                   uint32 &GltNoisePerlin::seed()    { return _seed;    }
                   uint32 &GltNoisePerlin::octaves() { return _octaves; }
                   uint32 &GltNoisePerlin::lambda()  { return _lambda;  }
                     real &GltNoisePerlin::omega()   { return _omega;   }

const GltNoisePerlin::NoiseType &GltNoisePerlin::type()    const { return _type;    }
const                    uint32 &GltNoisePerlin::seed()    const { return _seed;    }
const                    uint32 &GltNoisePerlin::octaves() const { return _octaves; }
const                    uint32 &GltNoisePerlin::lambda()  const { return _lambda;  }
const                      real &GltNoisePerlin::omega()   const { return _omega;   }

//

real
GltNoisePerlin::val(const real x) const
{
    return val(Vector(x,0,0));
}

real
GltNoisePerlin::val(const real x,const real y) const
{
    return val(Vector(x,y,0));
}

real
GltNoisePerlin::val(const real x,const real y,const real z) const
{
    return val(Vector(x,y,z));
}

real
GltNoisePerlin::val(const Vector &pos) const
{
    const uint32 x = (uint32) floor(pos.x());
    const uint32 y = (uint32) floor(pos.y());
    const uint32 z = (uint32) floor(pos.z());

    const real fx = pos.x()<1 ? fmod(pos.x(),1.0) + 1 : fmod(pos.x(),1.0);
    const real fy = pos.y()<1 ? fmod(pos.y(),1.0) + 1 : fmod(pos.y(),1.0);
    const real fz = pos.z()<1 ? fmod(pos.z(),1.0) + 1 : fmod(pos.z(),1.0);

    return val(x,y,z,0,fx,fy,fz);
}

real
GltNoisePerlin::val
(
    const uint32 x,
    const uint32 y,
    const uint32 z,
    const uint32 fractionalBits,
    const real   fx,
    const real   fy,
    const real   fz
) const
{
    switch (_type)
    {
        default:
        case NOISE_PERLIN:
            return perlinNoise(_seed,x,y,z,fractionalBits,fx,fy,fz);

        case NOISE_FBM:
        {
            const real sf = 1.0/(1<<_lambda);

            uint32 f     = fractionalBits;
            real   scale = 1.0;
            real   ffx   = fx;
            real   ffy   = fy;
            real   ffz   = fz;
            real   val   = perlinNoise(_seed,x,y,z,f,ffx,ffy,ffz);

            // Add subsequent harmonics

            if (_octaves>0)
                for (uint32 i=1; i<_octaves; i++)
                {
                    // <x,y,z>/2^lambda

                    f -= _lambda;

                    ffx *= sf;
                    ffy *= sf;
                    ffz *= sf;

                    // Add scaled harmonic

                    if (f>=0)
                    {
                        scale *= _omega;
                        val += perlinNoise(_seed^i,x,y,z,f,ffx,ffy,ffz)*scale;
                    }
                }

            return val;
        }

        case NOISE_TURBULENCE:
        {
            const real sf = 1.0/(1<<_lambda);

            uint32 f     = fractionalBits;
            real   scale = 1.0;
            real   ffx   = fx;
            real   ffy   = fy;
            real   ffz   = fz;
            real   val   = fabs(perlinNoise(_seed,x,y,z,f,ffx,ffy,ffz));

            // Add subsequent harmonics

            if (_octaves>0)
                for (uint32 i=1; i<_octaves; i++)
                {
                    // <x,y,z>/2^lambda

                    f -= _lambda;

                    ffx *= sf;
                    ffy *= sf;
                    ffz *= sf;

                    // Add scaled harmonic

                    if (f>=0)
                    {
                        scale *= _omega;
                        val += fabs(perlinNoise(_seed^i,x,y,z,f,ffx,ffy,ffz)*scale);
                    }
                }

            return val;
        }
    }
}

real
GltNoisePerlin::val
(
    const uint32 x,
    const uint32 y,
    const uint32 fractionalBits,
    const real   fx,
    const real   fy
) const
{
    switch (_type)
    {
        default:
        case NOISE_PERLIN:
            return perlinNoise(_seed,x,y,fractionalBits,fx,fy);

        case NOISE_FBM:
        {
            const real sf = 1.0/(1<<_lambda);

            uint32 f     = fractionalBits;
            real   scale = 1.0;
            real   ffx   = fx;
            real   ffy   = fy;
            real   val   = perlinNoise(_seed,x,y,f,ffx,ffy);

            // Add subsequent harmonics

            if (_octaves>0)
                for (uint32 i=1; i<_octaves; i++)
                {
                    // <x,y,z>/2^lambda

                    f -= _lambda;

                    ffx *= sf;
                    ffy *= sf;

                    // Add scaled harmonic

                    if (f>=0)
                    {
                        scale *= _omega;
                        val += perlinNoise(_seed^i,x,y,f,ffx,ffy)*scale;
                    }
                }

            return val;
        }

        case NOISE_TURBULENCE:
        {
            const real sf = 1.0/(1<<_lambda);

            uint32 f     = fractionalBits;
            real   scale = 1.0;
            real   ffx   = fx;
            real   ffy   = fy;
            real   val   = fabs(perlinNoise(_seed,x,y,f,ffx,ffy));

            // Add subsequent harmonics

            if (_octaves>0)
                for (uint32 i=1; i<_octaves; i++)
                {
                    // <x,y,z>/2^lambda

                    f -= _lambda;

                    ffx *= sf;
                    ffy *= sf;

                    // Add scaled harmonic

                    if (f>=0)
                    {
                        scale *= _omega;
                        val += fabs(perlinNoise(_seed^i,x,y,f,ffx,ffy)*scale);
                    }
                }

            return val;
        }
    }
}

real
GltNoisePerlin::val
(
    const uint32 x,
    const uint32 fractionalBits,
    const real   fx
) const
{
    switch (_type)
    {
        default:
        case NOISE_PERLIN:
            return perlinNoise(_seed,x,fractionalBits,fx);

        case NOISE_FBM:
        {
            const real sf = 1.0/(1<<_lambda);

            uint32 f     = fractionalBits;
            real   scale = 1.0;
            real   ffx   = fx;
            real   val   = perlinNoise(_seed,x,f,ffx);

            // Add subsequent harmonics

            if (_octaves>0)
                for (uint32 i=1; i<_octaves; i++)
                {
                    // <x,y,z>/2^lambda

                    f -= _lambda;

                    ffx *= sf;

                    // Add scaled harmonic

                    if (f>=0)
                    {
                        scale *= _omega;
                        val += perlinNoise(_seed^i,x,f,ffx)*scale;
                    }
                }

            return val;
        }

        case NOISE_TURBULENCE:
        {
            const real sf = 1.0/(1<<_lambda);

            uint32 f     = fractionalBits;
            real   scale = 1.0;
            real   ffx   = fx;
            real   val   = fabs(perlinNoise(_seed,x,f,ffx));

            // Add subsequent harmonics

            if (_octaves>0)
                for (uint32 i=1; i<_octaves; i++)
                {
                    // <x,y,z>/2^lambda

                    f -= _lambda;

                    ffx *= sf;

                    // Add scaled harmonic

                    if (f>=0)
                    {
                        scale *= _omega;
                        val += fabs(perlinNoise(_seed^i,x,f,ffx)*scale);
                    }
                }

            return val;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

real
GltNoisePerlin::perlinNoise
(
    const uint32 seed,
    const uint32 x,
    const uint32 fractionalBits,
    const real   fx
)
{
    // Some sanity checking in debug mode

    assert(fx>=0.0 && fx<=1.0);
    assert(fractionalBits<32);

    // Determine the integer co-ordinates of the lattice
    // interval to be interpolated.  Note that the lattice
    // tiles seamlessly and fractionalBits are discarded.

    const uint32 px[2] = { x>>fractionalBits, (x+(1<<fractionalBits))>>fractionalBits };

    // The floating-point fx displacement is
    // scaled down to be less significant than the
    // fractional components of x,y and z

    const real subFraction = 1/real(1<<fractionalBits);

    // Combine the fractional bits of x and y
    // with the floating-point displacements
    // to determine the location within the square

    real fx0;
    fraction(fx0,(x<<(32-fractionalBits)));
    fx0 += fx*subFraction;

    // Determine the complement value of fx0

    const real fx1 = fx0-1;

    // Apply ease-in ease-out curve, rather
    // than interpolating linearly

    const real wx = scurve(fx0);

    // Temporary storage for interpolation

    uint32 i;
    real vx0, vx1;

    // Interpolate along edge from origin in +x direction

    i = hash(seed,px[0]);
    vx0 = _vector[i].x()*fx0;
    i = hash(seed,px[1]);
    vx1 = _vector[i].x()*fx1;

    return vx0 + wx*(vx1-vx0);
}

real
GltNoisePerlin::perlinNoise
(
    const uint32 seed,
    const uint32 x,
    const uint32 y,
    const uint32 fractionalBits,
    const real   fx,
    const real   fy
)
{
    // Some sanity checking in debug mode

    assert(fx>=0.0 && fx<=1.0);
    assert(fy>=0.0 && fy<=1.0);
    assert(fractionalBits<32);

    // Determine the integer co-ordinates of the lattice
    // square to be interpolated.  Note that the lattice
    // tiles seamlessly and fractionalBits are discarded.

    const uint32 px[2] = { x>>fractionalBits, (x+(1<<fractionalBits))>>fractionalBits };
    const uint32 py[2] = { y>>fractionalBits, (y+(1<<fractionalBits))>>fractionalBits };

    // The floating-point fx and fy displacements are
    // scaled down to be less significant than the
    // fractional components of x,y and z

    const real subFraction = 1/real(1<<fractionalBits);

    // Combine the fractional bits of x and y
    // with the floating-point displacements
    // to determine the location within the square

    real fx0;
    fraction(fx0,(x<<(32-fractionalBits)));
    fx0 += fx*subFraction;

    real fy0;
    fraction(fy0,(y<<(32-fractionalBits)));
    fy0 += fy*subFraction;

    // Determine the complement values of fx0 and fy0

    const real fx1 = fx0-1;
    const real fy1 = fy0-1;

    // Apply ease-in ease-out curve, rather
    // than interpolating linearly

    const real wx = scurve(fx0);
    const real wy = scurve(fy0);

    // Temporary storage for interpolation

    uint32 i;
    real vx0, vx1, vy0, vy1;

    // Interpolate along edge from origin in +x direction

    i = hash(seed,px[0],py[0]);
    vx0 = _vector[i].x()*fx0 + _vector[i].y()*fy0;
    i = hash(seed,px[1],py[0]);
    vx1 = _vector[i].x()*fx1 + _vector[i].y()*fy0;
    vy0 = vx0 + wx*(vx1-vx0);

    // Interpolate along edge from +y in +x direction

    i = hash(seed,px[0],py[1]);
    vx0 = _vector[i].x()*fx0 + _vector[i].y()*fy1;
    i = hash(seed,px[1],py[1]);
    vx1 = _vector[i].x()*fx1 + _vector[i].y()*fy1;
    vy1 = vx0 + wx*(vx1-vx0);

    // Interpolate between the two edges
    return vy0 + wy*(vy1-vy0);
}

real
GltNoisePerlin::perlinNoise
(
    const uint32 seed,
    const uint32 x,
    const uint32 y,
    const uint32 z,
    const uint32 fractionalBits,
    const real   fx,
    const real   fy,
    const real   fz
)
{
    // Some sanity checking in debug mode

    assert(fx>=0.0 && fx<=1.0);
    assert(fy>=0.0 && fy<=1.0);
    assert(fz>=0.0 && fz<=1.0);
    assert(fractionalBits<32);

    // Determine the integer co-ordinates of the lattice
    // cube to be interpolated.  Note that the lattice
    // tiles seamlessly and fractionalBits are discarded.

    const uint32 px[2] = { x>>fractionalBits, (x+(1<<fractionalBits))>>fractionalBits };
    const uint32 py[2] = { y>>fractionalBits, (y+(1<<fractionalBits))>>fractionalBits };
    const uint32 pz[2] = { z>>fractionalBits, (z+(1<<fractionalBits))>>fractionalBits };

    // The floating-point fx,fy,fz displacements are
    // scaled down to be less significant than the
    // fractional components of x,y and z

    const real subFraction = 1/real(1<<fractionalBits);

    // Combine the fractional bits of x,y and z
    // with the floating-point displacements
    // to determine the location within the cube

    real fx0;
    fraction(fx0,(x<<(32-fractionalBits)));
    fx0 += fx*subFraction;

    real fy0;
    fraction(fy0,(y<<(32-fractionalBits)));
    fy0 += fy*subFraction;

    real fz0;
    fraction(fz0,(z<<(32-fractionalBits)));
    fz0 += fz*subFraction;

    // Determine the complement values of fx0,fy0 and fz0

    const real fx1 = fx0-1;
    const real fy1 = fy0-1;
    const real fz1 = fz0-1;

    // Apply ease-in ease-out curve, rather
    // than interpolating linearly

    const real wx = scurve(fx0);
    const real wy = scurve(fy0);
    const real wz = scurve(fz0);

    // Temporary storage for interpolation

    uint32 i;
    real vx0, vx1, vy0, vy1, vz0, vz1;

    // Interpolate along edge from origin in +x direction

    i = hash(seed,px[0],py[0],pz[0]);
    vx0 = _vector[i].x()*fx0 + _vector[i].y()*fy0 + _vector[i].z()*fz0;
    i = hash(seed,px[1],py[0],pz[0]);
    vx1 = _vector[i].x()*fx1 + _vector[i].y()*fy0 + _vector[i].z()*fz0;
    vy0 = vx0 + wx*(vx1-vx0);

    // Interpolate along edge from +y in +x direction

    i = hash(seed,px[0],py[1],pz[0]);
    vx0 = _vector[i].x()*fx0 + _vector[i].y()*fy1 + _vector[i].z()*fz0;
    i = hash(seed,px[1],py[1],pz[0]);
    vx1 = _vector[i].x()*fx1 + _vector[i].y()*fy1 + _vector[i].z()*fz0;
    vy1 = vx0 + wx*(vx1-vx0);

    // Interpolate between the two edges
    vz0 = vy0 + wy*(vy1-vy0);

    // Interpolate along edge from +z in +x direction

    i = hash(seed,px[0],py[0],pz[1]);
    vx0 = _vector[i].x()*fx0 + _vector[i].y()*fy0 + _vector[i].z()*fz1;
    i = hash(seed,px[1],py[0],pz[1]);
    vx1 = _vector[i].x()*fx1 + _vector[i].y()*fy0 + _vector[i].z()*fz1;
    vy0 = vx0 + wx*(vx1-vx0);

    // Interpolate along edge from +y+z in +x direction

    i = hash(seed,px[0],py[1],pz[1]);
    vx0 = _vector[i].x()*fx0 + _vector[i].y()*fy1 + _vector[i].z()*fz1;
    i = hash(seed,px[1],py[1],pz[1]);
    vx1 = _vector[i].x()*fx1 + _vector[i].y()*fy1 + _vector[i].z()*fz1;
    vy1 = vx0 + wx*(vx1-vx0);

    // Interpolate between the two +z edges
    vz1 = vy0 + wy*(vy1-vy0);

    // Finally, interpolate betwen the two planes
    return vz0 + wz*(vz1-vz0);
}

