// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2020 Gwenhael Goavec-Merou <gwenhael.goavec-merou@trabucayre.com>
 */

#ifndef JTAG_H
#define JTAG_H

#include <iostream>
#include <string>
#include <vector>

#include "board.hpp"
#include "cable.hpp"
#include "jtagInterface.hpp"
#include "part.hpp"

class Jtag {
 public:
	Jtag(const cable_t &cable, const jtag_pins_conf_t *pin_conf,
		const std::string &dev,
		const std::string &serial, uint32_t clkHZ, int8_t verbose,
		const std::string &ip_adr, int port,
		const bool invert_read_edge = false,
		const std::string &firmware_path = "");
	~Jtag();

	/* maybe to update */
	int setClkFreq(uint32_t clkHZ) { return _jtag->setClkFreq(clkHZ);}
	uint32_t getClkFreq() { return _jtag->getClkFreq();}

	typedef struct {
		uint32_t idcode;
		uint16_t irlength;
		bool is_misc;
		device_model *model;
	} found_device;

	/*!
	 * \brief scan JTAG chain to obtain IDCODE. Fill
	 *        a vector with all idcode and another
	 *        vector with irlength
	 * \return number of devices found
	 */
	int detectChain(int max_dev);

	/*!
	 * \brief return list of devices in the chain
	 * \return list of devices
	 */
	//std::vector<int> get_devices_list() {return _devices_list;}
	std::vector<found_device> get_devices_list() {return _f_device_list;}

	/*!
	 * \brief return current selected device idcode
	 * \return device idcode
	 */
	uint32_t get_target_device_id() {return _f_device_list[device_index].idcode;}

	/*!
	 * \brief set index for targeted FPGA
	 * \param[in] index: index in the chain
	 * \return -1 if index is out of bound, index otherwise
	 */
	uint16_t device_select(uint16_t index);
	/*!
	 * \brief inject a device into list at the begin
	 * \param[in] device_id: idcode
	 * \param[in] irlength: device irlength
	 * \return false if fails
	 */
	//bool insert_first(uint32_t device_id, uint16_t irlength);
	bool insert_first(uint32_t idcode, bool is_misc, uint16_t irlength, device_model *device);

	int get_nb_targets() {
		int nb = 0;
		for (auto t: _f_device_list)
			if (!t.is_misc)
				nb++;
		return nb;
	}

	/*!
	 * \brief return a pointer to the transport subclass
	 * \return a pointer instance of JtagInterface
	 */
	JtagInterface *get_ll_class() {return _jtag;}

	int shiftIR(unsigned char *tdi, unsigned char *tdo, int irlen,
		int end_state = RUN_TEST_IDLE);
	int shiftIR(unsigned char tdi, int irlen,
		int end_state = RUN_TEST_IDLE);
	int shiftDR(unsigned char *tdi, unsigned char *tdo, int drlen,
		int end_state = RUN_TEST_IDLE);
	int read_write(unsigned char *tdi, unsigned char *tdo, int len, char last);

	void toggleClk(int nb);
	void go_test_logic_reset();
	void set_state(int newState);
	int flushTMS(bool flush_buffer = false);
	void flush() {flushTMS(); _jtag->flush();}
	void setTMS(unsigned char tms);

	enum tapState_t {
		TEST_LOGIC_RESET = 0,
		RUN_TEST_IDLE = 1,
		SELECT_DR_SCAN = 2,
		CAPTURE_DR = 3,
		SHIFT_DR = 4,
		EXIT1_DR = 5,
		PAUSE_DR = 6,
		EXIT2_DR = 7,
		UPDATE_DR = 8,
		SELECT_IR_SCAN = 9,
		CAPTURE_IR = 10,
		SHIFT_IR = 11,
		EXIT1_IR = 12,
		PAUSE_IR = 13,
		EXIT2_IR = 14,
		UPDATE_IR = 15,
		UNKNOWN = 999
	};
	const char *getStateName(tapState_t s);

	/* utilities */
	void setVerbose(int8_t verbose){_verbose = verbose;}

	JtagInterface *_jtag;


 private:
	/*!
	 * \brief search in fpga_list and misc_dev_list for a device with idcode
	 *        if found insert idcode and irlength in _devices_list and
	 *        _irlength_list
	 * \param[in] idcode: device idcode
	 * \return false if not found, true otherwise
	 */
	bool search_and_insert_device_with_idcode(uint32_t idcode);
	bool _verbose;
	int _state;
	int _tms_buffer_size;
	int _num_tms;
	unsigned char *_tms_buffer;
	std::string _board_name;

	int device_index; /*!< index for targeted FPGA */
	//std::vector<int32_t> _devices_list; /*!< ordered list of devices idcode */
	std::vector<int16_t> _irlength_list; /*!< ordered list of irlength */
	std::vector<found_device> _f_device_list;
};
#endif
