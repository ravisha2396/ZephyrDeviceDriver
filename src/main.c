#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <shell/shell.h>
#include <fsl_lpspi.h>
#include <shell/shell_uart.h>
#include <version.h>
#include <stdlib.h>
#include <drivers/pwm.h>
#include <drivers/display.h>
#include <fsl_common.h>
//#include <fsl_gpio.h> 
#include <fsl_iomuxc.h>
//#include <soc.h>

#define DEBUG 

#if defined(DEBUG) 
	#define DPRINTK(fmt, args...) printk("DEBUG: %s():%d: " fmt, \
   		 __func__, __LINE__, ##args)
#else
 	#define DPRINTK(fmt, args...) /* do nothing if not defined*/
#endif

// GET PROPERTIES OF PWM DEVICE FROM DEVICE TREE

#define PWM_LEDR_NODE	DT_NODELABEL(r_pwm)

#define R_CHANNEL	DT_PWMS_CHANNEL(PWM_LEDR_NODE)	

#define PWM_LEDG_NODE	DT_NODELABEL(g_pwm)

#define G_CHANNEL	DT_PWMS_CHANNEL(PWM_LEDG_NODE)	

#define PWM_LEDB_NODE	DT_NODELABEL(b_pwm)

#define B_CHANNEL	DT_PWMS_CHANNEL(PWM_LEDB_NODE)

// GET PROPERTIES OF DISPLAY DEVICE FROM DEVICE TREE

#define MAX_COUNT DT_PROP(DT_NODELABEL(disp_node),width)

#define FREQUENCY 50U // PWM WAVE FREQUENCY

#define PERIOD USEC_PER_SEC/FREQUENCY // PWM WAVE TIME PERIOD

const struct device *pwm1, *pwm2, *pwm3; // DEVICE POINTERS FOR PWM DEVICES

static int x,y,z; 

static uint8_t data[MAX_COUNT]={0}; 

int n,row,height=0;

volatile bool blink_on;

const struct device *disp; 

static int cmd_cycle_pwm(const struct shell *shell, size_t argc, char **argv)
{		
		
		int ret;

		x=atoi(argv[1]);
		y=atoi(argv[2]);
		z=atoi(argv[3]);
	
	if(argc!=4){
		shell_print(shell,"Invalid number of arguments!");
		return -1;
	}
		// TRIGGER PWM WAVE ON RED LED USING PWM API AND DUTY CYCLE RECIEVED IN SHELL ARGUMENTS
		ret=pwm_pin_set_usec(pwm1, R_CHANNEL, PERIOD, x*(PERIOD/100),0);
		
		if(ret){
			printk("Error %d: failed to set pulse width\n", ret);
			return ret;
		}
		k_usleep(PERIOD);

		// TRIGGER PWM WAVE ON GREEN LED USING PWM API AND DUTY CYCLE RECIEVED IN SHELL ARGUMENTS
		ret=pwm_pin_set_usec(pwm2, G_CHANNEL, PERIOD, y*(PERIOD/100), 0);
		
		if(ret){
			printk("Error %d: failed to set pulse width\n", ret);
			return ret;
		}

		// TRIGGER PWM WAVE ON BLUEUSING PWM API AND DUTY CYCLE RECIEVED IN SHELL ARGUMENTS
		ret=pwm_pin_set_usec(pwm3, B_CHANNEL, PERIOD, z*(PERIOD/100), 0);
		
		if(ret){
			printk("Error %d: failed to set pulse width\n", ret);
			return ret;
		}
	// --------------------  PWM CODE ENDS --------------------------	
	return 0;
}

static int cmd_blinking_disp(const struct shell *shell, size_t argc, char **argv)
{
	n=atoi(argv[1]);

	// CHECK IF NUMBER OF ARGUMENTS ARE VALID

	if(argc!=2){
		shell_print(shell,"Invalid number of arguments!");
		return -1;
	}

	if(n==1)
		blink_on=true;
	else if(n==0){
		blink_on=false;
		display_blanking_off(disp);
	}

	// IF n IS ANY OTHER VALUE THEN PRINT ERROR 
	else{
		shell_print(shell,"Invalid number of arguments!");
		return -1;	
	}
		
	return 0;
}

static int cmd_pattern(const struct shell *shell, size_t argc, char **argv)
{	
		row=atoi(argv[1]);
		if((argc>11) || (argc<3) || row < 0 || row > 7){
			shell_print(shell,"Invalid number of arguments!");
			return -1;	
		}
		height = argc-2;
		// do height check
		if(height > MAX_COUNT-row){
			shell_print(shell,"Invalid number of arguments!");
			return -1;
		}

		int k=2;
		for(int i=row; i<row+height; i++){
			data[i]=strtol(argv[k], NULL, 16);
			k++;
		}

		// USE BUFFER DESCRIPTOR TO PASS DESCRIPTOR BUFFER PROPERTIES

		struct display_buffer_descriptor desc;

		desc.buf_size=sizeof(data);
		desc.width=MAX_COUNT;
		desc.height= height;
		desc.pitch=MAX_COUNT;
		
	// ledm operation
		display_write(disp, 0, row, &desc, &data);	
			
	return 0;
}

// SUBCOMMAND SET WITH CALL BACK FUNCTIONS

SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo,
	SHELL_CMD(rgb, NULL, "Sets duty cycle for PWM pulses", cmd_cycle_pwm),
	SHELL_CMD(ledm, NULL, "Displays a pattern on LED Matrix", cmd_pattern),
	SHELL_CMD(ledb, NULL, "Used for blinking a pattern on LED Matrix", cmd_blinking_disp),
	SHELL_SUBCMD_SET_END // Array terminated. //
);

// REGISTER SHELL COMMAND p2 WITH SUBCOMMAND SET SUB_DEMO

SHELL_CMD_REGISTER(p2, &sub_demo, "Root command for LED operations", NULL);


void main(void)
{	
	// PIN MUX OPERATIONS FOR SETTING PINS OF GPIO DEVICE

	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_11_FLEXPWM1_PWMB03, 0);

	IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_11_FLEXPWM1_PWMB03,
			    IOMUXC_SW_PAD_CTL_PAD_PUE(1) |
			    IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
			    IOMUXC_SW_PAD_CTL_PAD_SPEED(2) |
			    IOMUXC_SW_PAD_CTL_PAD_DSE(6));

	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_10_FLEXPWM1_PWMA03, 0);

	IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_10_FLEXPWM1_PWMA03,
			    IOMUXC_SW_PAD_CTL_PAD_PUE(1) |
			    IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
			    IOMUXC_SW_PAD_CTL_PAD_SPEED(2) |
			    IOMUXC_SW_PAD_CTL_PAD_DSE(6));

	IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01, 0);

	IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_FLEXPWM1_PWMB01,
			    IOMUXC_SW_PAD_CTL_PAD_PUE(1) |
			    IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
			    IOMUXC_SW_PAD_CTL_PAD_SPEED(2) |
			    IOMUXC_SW_PAD_CTL_PAD_DSE(6));

	// PIN MUX OPERATIONS FOR SETTING MOSI, CS AND CLK PINS FOR LPSPI DEVICE

	IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO, 0);

	IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO,
			    IOMUXC_SW_PAD_CTL_PAD_PUE(1) |
			    IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
			    IOMUXC_SW_PAD_CTL_PAD_SPEED(2) |
			    IOMUXC_SW_PAD_CTL_PAD_DSE(6));
	
	IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK, 0);

	IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK,
			    IOMUXC_SW_PAD_CTL_PAD_PUE(1) |
			    IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
			    IOMUXC_SW_PAD_CTL_PAD_SPEED(2) |
			    IOMUXC_SW_PAD_CTL_PAD_DSE(6));

	IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_01_LPSPI1_PCS0, 0);

	IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_LPSPI1_PCS0,
			    IOMUXC_SW_PAD_CTL_PAD_PUE(1) |
			    IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
			    IOMUXC_SW_PAD_CTL_PAD_SPEED(2) |
			    IOMUXC_SW_PAD_CTL_PAD_DSE(6));
	
	// --------------------  PWM CODE BEGINS --------------------------

	char* n1=DT_PROP(DT_NODELABEL(flexpwm1_pwm3),label);
	
	pwm1 = device_get_binding(n1);
	if (!pwm1) {
		printk("Error: PWM device %s is not ready\n", pwm1->name);
		return;
	}
	
	char* n2=DT_PROP(DT_NODELABEL(flexpwm1_pwm3),label);
	
	pwm2 = device_get_binding(n2);
	if (!pwm2) {
		printk("Error: PWM device %s is not ready\n", pwm2->name);
		return;
	}
	
	char* n3=DT_PROP(DT_NODELABEL(flexpwm1_pwm1),label);
	
	pwm3 = device_get_binding(n3);
	if (!pwm3) {
		printk("Error: PWM device %s is not ready\n", pwm3->name);
		return;
	}	
	// --------------------PWM CODE end ---------------------------------------------

	// --------------------- DISPLAY code start -------------------------------------------			
	disp = device_get_binding(DT_PROP(DT_NODELABEL(disp_node),label));
	
		while(1){
			if(blink_on){
			display_blanking_on(disp);
			k_msleep(1000);
			display_blanking_off(disp);
			k_msleep(1000);
		}
		k_msleep(1);
		}
	// --------------------- DISPLAY code end -------------------------------------------
}


