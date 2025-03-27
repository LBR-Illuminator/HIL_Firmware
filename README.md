# Wiseled_LBR Hardware-in-the-Loop (HIL) Testing System

## Project Overview

The Wiseled_LBR HIL (Hardware-in-the-Loop) testing system is a comprehensive validation framework for the Wiseled smart lighting ecosystem. This project provides an advanced testing platform to simulate, validate, and verify the functionality of the Wiseled lamp system under various operational conditions.

### Project Components

- **Hardware Platform**: Nucleo-F446ZE Development Board
- **Testing Capabilities**: 
  - Signal generation
  - Signal measurement
  - Communication protocol simulation
  - Error condition testing

## System Architecture

### Signal Simulation Channels

#### Current Simulation
- 3 Independent Channels
- Range: 0-33 Amps
- Resolution: 10-bit (1024 levels)
- Voltage Scaling: 0-3.3V
- Sensitivity: 10 Amps per Volt

#### Temperature Simulation
- 3 Independent Channels
- Range: 0-330°C
- Resolution: 10-bit (1024 levels)
- Voltage Scaling: 0-3.3V
- Sensitivity: 100°C per Volt

#### PWM Input Capture
- 3 Channels
- 16-bit Resolution
- Capture on Both Edges

## Getting Started

### Prerequisites

- STM32CubeIDE
- Nucleo-F446ZE Development Board
- ST-Link Debugger

### Installation

1. Clone the repository
```bash
git clone https://github.com/your-organization/Wiseled_LBR_HIL.git
```

2. Open the project in STM32CubeIDE
   - File > Open Projects from File System
   - Select the project directory

3. Build the project
   - Project > Build All

### Running Tests

#### Basic Simulation
- Connect the Nucleo board
- Load the firmware
- Debug the application to run simulation scenarios

#### Test Scenarios
- Normal Operation Simulation
- Boundary Condition Testing
- Error Condition Validation

## Key Modules

### HIL Drivers (`hil_drivers.h`)
- Signal Generation
- Signal Measurement
- Error Handling
- Simulation Scenarios

### Peripheral Configuration
- USART3 Communication
- TIM1: PWM Input Capture
- TIM2: Current Signal Generation
- TIM3: Temperature Signal Simulation

## Simulation Capabilities

- Individual Light Source Testing
- Current Limit Verification
- Temperature Threshold Checking
- PWM Signal Characterization
- Communication Protocol Validation

## Debugging and Monitoring

- Use SWO (Serial Wire Output) for logging
- Utilize STM32CubeIDE's debug features
- Set breakpoints for detailed analysis

## Error Handling

The system implements comprehensive error detection and logging:
- Signal out-of-range detection
- Communication error simulation
- Thermal and electrical safety checks

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Licensing

Distributed under the [Your License]. See `LICENSE` for more information.

## Contact

Project Maintainer: [Your Name]
Project Link: [https://github.com/your-organization/Wiseled_LBR_HIL]

## Acknowledgments

- STMicroelectronics
- Nucleo Development Board Team
- Open-source Community

---

**Note**: This is a test framework for the Wiseled_LBR smart lighting system. Actual implementation may vary based on specific requirements and testing scenarios.