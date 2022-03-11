#include "rb.h"

void rb_init(rb_handle_t * rb, uint32_t size)
{
	FASSERT(size > 0 && size % 4 == 0);

	rb->size = size;
	rb->length = 0;

	rb->write_index = 0;
	rb->read_index = 0;

	rb->data = malloc(size);
}

void rb_free(rb_handle_t * rb)
{
	free(rb->data);
}


uint32_t rb_read(rb_handle_t * rb, uint32_t len, uint8_t * * data)
{
	UBaseType_t uxSavedInterruptStatus;
	if (xPortIsInsideInterrupt())
	{
		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	}
	else
	{
		taskENTER_CRITICAL();
	}

	(*data) = rb->data + rb->read_index;

	if (len > rb->length)
		len = rb->length;

	if (rb->read_index + len > rb->size)
		len = rb->size - rb->read_index;

	rb->read_index = (rb->read_index + len) % rb->size;
	rb->length -= len;

	if (xPortIsInsideInterrupt())
	{
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	}
	else
	{
		taskEXIT_CRITICAL();
	}

	return len;
}

bool rb_write(rb_handle_t * rb, uint32_t len, uint8_t * data)
{
	UBaseType_t uxSavedInterruptStatus;
	if (xPortIsInsideInterrupt())
	{
		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	}
	else
	{
		taskENTER_CRITICAL();
	}

	bool ret = true;

	if (rb->size - rb->length < len)
	{
		ret = false;
	}
	else
	{
		if (rb->write_index + len > rb->size)
		{
			uint16_t first_len = rb->size - rb->write_index;
			memcpy(rb->data + rb->write_index, data, first_len);
			memcpy(rb->data, data + first_len, len - first_len);
		}
		else
		{
			memcpy(rb->data + rb->write_index, data, len);
		}

		rb->write_index = (rb->write_index + len) % rb->size;
		rb->length += len;
	}

	if (xPortIsInsideInterrupt())
	{
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	}
	else
	{
		taskEXIT_CRITICAL();
	}

	return ret;
}

uint32_t rb_length(rb_handle_t * rb)
{
	return rb->length;
}

void rb_clear(rb_handle_t * rb)
{
	UBaseType_t uxSavedInterruptStatus;
	if (IS_IRQ_MODE())
	{
		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	}
	else
	{
		taskENTER_CRITICAL();
	}

	rb->length = 0;
	rb->write_index = 0;
	rb->read_index = 0;

	if (IS_IRQ_MODE())
	{
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	}
	else
	{
		taskEXIT_CRITICAL();
	}
}
