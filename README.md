# STM8 clock firmware for IRCON themometer nixies

This is the firmware for my Hackaday project [Repurposing an old nixie thermometer](https://hackaday.io/project/183352-repurposing-an-old-nixie-thermometer). It is based on an earlier project [Irreproducible clock](https://hackaday.io/project/175456-irreproducible-clock). Instead of driving multiplexed 7-segment LED displays, the [board](https://hackaday.io/project/184120-quick-and-simple-stm8-board-with-tcxo) drives a [3 line to 32 line output expander](https://hackaday.io/project/184239-3-line-to-32-line-output-expander) which drives the nixies.

## Versioning

First release April 2022

## Notes

You will need the Standard Peripherals Library, explained [here](https://github.com/retiredfeline/STM8-SPL-SDCC/wiki/Instructions-for-building-SPL-on-Linux) and the include file from [Protothreads](http://dunkels.com/adam/pt/).

## Authors

* **Ken Yap**

## License

See the [LICENSE](LICENSE.md) file for license rights and limitations (MIT).
