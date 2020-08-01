void vpu_update_ly() {
	device.io[IO_STAT] = (device.io[IO_STAT] & ~0x04) | ((device.io[IO_LY] == device.io[IO_LYC]) ? 0x04 : 0x00);
	if (device.io[IO_STAT] & 0x04) {
		int x = 0;
	}
	if ((device.io[IO_STAT] & 0x44) == 0x44 && !device.vpu.disabled) {
		device_request_interrupt(INT_LCDC);
	}
}

void vpu_reset_ly() {
	device.io[IO_LY] = 0;
	vpu_update_ly();
	if (!device.vpu.disabled) {
		device_request_interrupt(INT_LCDC);
		gbv_mode2();
	}
}

void vpu_clock() {
	if (device.io[IO_LCDC] & GBV_LCDC_CTRL) {
		device.vpu.disabled = false;
		switch (gbv_stat_mode()) {
		case GBV_LCD_MODE_HBLANK:
			if (device.vpu.cycles > 207) {
				device.vpu.cycles = 0;
				device.io[IO_LY]++;
				if (device.io[IO_LY] == GBV_SCREEN_HEIGHT) {
					gbv_mode1();
				}
				else {
					gbv_mode2();
				}
			}
			break;
		case GBV_LCD_MODE_VBLANK:
			if (device.io[IO_LY] < 153) {
				if (device.vpu.cycles > 520) {
					device.io[IO_LY]++;
					device.vpu.cycles = 0;
				}
			}
			else {
				device.vpu.cycles = 0;
				device.io[IO_LY] = 0;
				gbv_mode2();
			}
			break;
		case GBV_LCD_MODE_OAM:
			if (device.vpu.cycles > 83) {
				device.vpu.cycles = 0;
				gbv_mode3();
			}
			break;
		case GBV_LCD_MODE_TRANSFER:
			if (device.vpu.cycles > 175) {
				device.vpu.cycles = 0;
				gbv_render_scanline((u8*)device.render_buffer, &device.palette);
				gbv_mode0();
			}
			break;
		}
		vpu_update_ly();
		device.vpu.cycles++;
	}
	else if (!device.vpu.disabled) {
		device.vpu.disabled = true;
		device.io[IO_STAT] = device.io[IO_STAT] & 0xf8;
		device.io[IO_LY] = 0;
		vpu_update_ly();
	}
}