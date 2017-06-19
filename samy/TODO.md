# Priority
- [x] Strip out all gLow related code
- [x] Run scene with scanned objs
- [ ] Set scene underwater
    * Research light scattering (increase Mie scattering?) [1](http://www.iquilezles.org/www/articles/fog/fog.htm) [2](https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html)
    * [Color Underwater](http://www.deep-six.com/page77.htm)
- [ ] Caustics + god rays
- [ ] Bubbles
- [ ] Integrate PRM paths

# Look into
- [ ] Data visualization
    * Vector field from DVL
    * HUD with sensor info (temperature, salinity, etc.)
- [ ] Heightmap from bathymetry data
- [ ] Remove bloom?
- [Ocean Rendering](https://outerra.blogspot.com.mt/2011/02/ocean-rendering.html)
- [Using Vertex Texture Displacement for Realistic Water Rendering](https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter18.html)

# Extra
- [ ] Switch to Lua scripts
    * more flexible and automated placement
    * console to adjust parameters on the fly (imgui has widget for this)
- [ ] Port all Eigen code to GLM
- [ ] Clang format
