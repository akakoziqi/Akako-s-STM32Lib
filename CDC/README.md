# STM32 CDC虚拟串口

在usbd_cdc_if.c中增加了usb_printf()函数，可用于直接打印虚拟串口信息

## 使用虚拟串口步骤：

1. 在CubeMX中的Connectivity栏中将USB功能启用

2. 在Middleware栏中将USB_DEVICE启用，并将模式改为VPC

3. 调节时钟树，USB的时钟最好从外部高速晶振分频得到，将速率设为48MHz

4. 修改usbd_cdc_if.c文件，在其中添加重定向函数：

   ```c
   /* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
   
   #include <stdarg.h>
    
   void usb_printf(const char *format, ...)
   {
       va_list args;
       uint32_t length;
    
       va_start(args, format);
       length = vsnprintf((char *)UserTxBufferFS, APP_TX_DATA_SIZE, (char *)format, args);
       va_end(args);
       CDC_Transmit_FS(UserTxBufferFS, length);
   }
   /* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
   ```

5. 然后把函数声明添加到usbd_cdc_if.h中

   ```c
   void usb_printf(const char *format, …);
   ```

6. 在头文件中添加头文件

   ```c
   #include "usbd_cdc_if.h"
   ```

   





