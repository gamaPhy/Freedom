## Electrical stuff

### TODO
- [ ] LEDs
  - [ ] Power with VSYS or VBUS?
  - [ ] Add addressable LED footprints to pcb
    - [ ] Choose suitable LEDs
- [ ] ADC
  - [ ] analog and digital ground plane placement
  - [ ] option to add external ADC
    - [ ] select suitable ICs
    - [ ] SPI or I2C?
    - [ ] Power with VSYS or VBUS?
    - [ ] option to use 3v3 or 5v to power? 
- [ ] HES (Hall effect sensors)
  - [ ] option to use 3v3 or 5v to power
  - [ ] What type of filter?
  - [ ] Power with VSYS or VBUS?
- [ ] Extra buttons for menu screen
- [ ] Add switch footprints to pcb
- [ ] Add extra switch with winding copper wire  around switch connected to ADC to test viability
- [ ] Rotary encoders with the internal ADC?

- SPI 
   - 0Ohm resistor terminations near source of  each line
- Debounce rotary encoder and buttons

Future:
- Find a suitable voltage regulator or voltage reference to accomodate powering all the sensors. 
  ~10mA maximum each * 16 = 160mA total. This might be too much load on 3v3 pin of pico.

