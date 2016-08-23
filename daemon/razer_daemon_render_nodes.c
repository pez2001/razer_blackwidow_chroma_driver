/* 
 * razer_chroma_drivers - a driver/tools collection for razer chroma devices
 * (c) 2015 by Tim Theede aka Pez2001 <pez2001@voyagerproject.de> / vp
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
 *
 */
#include "razer_daemon.h"

char *daemon_render_node_to_json(struct razer_fx_render_node *render_node)
{
	char *rn_json = str_CreateEmpty();
	rn_json = str_CatFree(rn_json,"\n {\n");
	rn_json = str_CatFree(rn_json," \"id\" : ");
	char *id_string = str_FromLong(render_node->id);
	rn_json = str_CatFree(rn_json,id_string);
	rn_json = str_CatFree(rn_json," ,\n");
	free(id_string);
	rn_json = str_CatFree(rn_json," \"name\": \"");
	rn_json = str_CatFree(rn_json,render_node->name);
	rn_json = str_CatFree(rn_json,"\" ,\n \"description\": \"");
	rn_json = str_CatFree(rn_json,render_node->description);

	rn_json = str_CatFree(rn_json,"\" ,\n \"effect_id\": \"");
	if(render_node->effect != NULL)
	{
		char *effect_id_string = str_FromLong(render_node->effect->id);
		rn_json = str_CatFree(rn_json, effect_id_string);
		free(effect_id_string);
	} else {
		rn_json = str_CatFree(rn_json,"None");
	}

	/*effect_json = str_CatFree(effect_json,"\" ,\n \"subs_num\" : ");
	char *parameters_num_string = str_FromLong(effect->parameters->num);
	effect_json = str_CatFree(effect_json,parameters_num_string);
	effect_json = str_CatFree(effect_json," ,\n");
	free(parameters_num_string);
	effect_json = str_CatFree(effect_json," \"parameters\" :  [\n");
	for(int i = 0;i<effect->parameters->num;i++)
	{
		struct razer_parameter *parameter = effect->parameters->items[i];
		char *parameter_json = daemon_parameter_to_json(parameter);
		effect_json = str_CatFree(effect_json,parameter_json);
		free(parameter_json);
	}*/
	rn_json = str_CatFree(rn_json,"\"\n },\n");
	return(rn_json);
}

int daemon_register_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	list_Push(daemon->fx_render_nodes,render_node);
	render_node->id = daemon->fx_render_nodes_uid++;
	return(render_node->id);
}

struct razer_fx_render_node *daemon_create_render_node(struct razer_chroma_device *device,struct razer_effect *effect,int input_render_node_uid,int second_input_render_node_uid,int output_render_node_uid,char *name,char *description)
{
	struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
	struct razer_fx_render_node *render_node = (struct razer_fx_render_node*)malloc(sizeof(struct razer_fx_render_node));
	render_node->device = device;
	render_node->daemon = (struct razer_daemon*)device_data->daemon;
	//render_node->effect = effect;
	if(effect)
		render_node->effect = daemon_create_effect_instance(device_data->daemon,effect);
	else
		render_node->effect = NULL;
	render_node->opacity = 1.0f;
	if(input_render_node_uid == -1)
	{
		struct razer_rgb_frame *iframe = razer_create_rgb_frame(device->columns_num,device->rows_num);
		render_node->input_frame = iframe;
		render_node->input_frame_linked_uid = -1;
	}
	else if(input_render_node_uid == 0) //set input to device output buffer
	{
		render_node->input_frame = device_data->frame_buffer;
		render_node->input_frame_linked_uid = 0;
	}
	else
	{
		struct razer_fx_render_node *rn = daemon_get_render_node(device_data->daemon,input_render_node_uid);
		render_node->input_frame = rn->output_frame;
		render_node->input_frame_linked_uid = input_render_node_uid;
	}

	if(second_input_render_node_uid == -1)
	{
		struct razer_rgb_frame *siframe = razer_create_rgb_frame(device->columns_num,device->rows_num);
		render_node->second_input_frame = siframe;
		render_node->second_input_frame_linked_uid = -1;
	}
	else if(second_input_render_node_uid == 0) //set input to device output buffer
	{
		render_node->second_input_frame = device_data->frame_buffer;
		render_node->second_input_frame_linked_uid = 0;
	}
	else
	{
		struct razer_fx_render_node *srn = daemon_get_render_node(device_data->daemon,second_input_render_node_uid);
		render_node->second_input_frame = srn->output_frame;
		render_node->second_input_frame_linked_uid = second_input_render_node_uid;
	}

	if(output_render_node_uid == -1)
	{
		struct razer_rgb_frame *oframe = razer_create_rgb_frame(device->columns_num,device->rows_num);
		render_node->output_frame = oframe;
		render_node->output_frame_linked_uid = -1;
	}
	else if(output_render_node_uid == 0) //set input to device output buffer
	{
		render_node->output_frame = device_data->frame_buffer;
		render_node->output_frame_linked_uid = 0;
	}
	/*else //not used
	{
		struct razer_fx_render_node *orn = daemon_get_render_node(daemon,output_render_node_uid);
		render_node->output_frame = orn->output_frame;
		render_node->output_frame_linked_uid = output_render_node_uid;
	}*/

	render_node->description = str_Copy(description);
	render_node->name = str_Copy(name);
	//render_node->fps = daemon->fps;
	render_node->compose_mode = RAZER_COMPOSE_MODE_MIX;
	render_node->next = NULL;
	render_node->parent = NULL;
	//render_node->parameters = NULL;
	//render_node->parameters_num = 0;
	render_node->subs = list_Create(0,0);
	render_node->start_ticks = 0;
	render_node->running = 0;//set to 1 with first update
	render_node->limit_render_time_ms = 0;
	render_node->move_frame_buffer_linkage_to_next = 1;
	//render_node->continue_chain=1;
	render_node->loop_count = -1;
	return(render_node);
}


struct razer_fx_render_node *daemon_get_render_node(struct razer_daemon *daemon,int uid)
{
	for(int i = 0;i<list_GetLen(daemon->fx_render_nodes);i++)
	{
		struct razer_fx_render_node *render_node = list_Get(daemon->fx_render_nodes,i);
		if(render_node->id == uid)
			return(render_node);
	}
	return(NULL);
}

void daemon_render_node_add_sub(struct razer_fx_render_node *render_node,struct razer_fx_render_node *sub_node)
{
	list_Push(render_node->subs,sub_node);
	sub_node->parent = render_node;
}


//int daemon_reset_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
int daemon_reset_render_node(struct razer_fx_render_node *render_node)
{
	if(!render_node)
		return(-1);
	if(!render_node->effect->reset)
		return(-1);
	int ret = render_node->effect->reset(render_node);
	return(ret);
}
 

int daemon_render_node_fire_parameter_changed(struct razer_fx_render_node *render_node,struct razer_parameter *parameter)
{
	if(!render_node)
		return(-1);
	if(!render_node->effect->parameter_changed)
		return(-1);
	int ret = render_node->effect->parameter_changed(render_node,parameter);
	return(ret);
}


void daemon_connect_frame_buffer(struct razer_chroma_device *device,struct razer_fx_render_node *render_node)
{
	struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
	device_data->is_render_nodes_dirty = 1;
	if(device_data->frame_buffer_linked_uid != 0) //unlink old render node first
	{
		struct razer_fx_render_node *old_rn = daemon_get_render_node(device_data->daemon,device_data->frame_buffer_linked_uid);
		old_rn->output_frame = razer_create_rgb_frame(device->columns_num,device->rows_num);
		old_rn->output_frame_linked_uid = -1;
	}
	if(!render_node)
		return;
	if(render_node->output_frame_linked_uid == -1)
		razer_free_rgb_frame(render_node->output_frame);
	render_node->output_frame = device_data->frame_buffer;
	device_data->frame_buffer_linked_uid = render_node->id;
	if (render_node->effect)
		device_data->daemon->fps = render_node->effect->fps;
}

void daemon_disconnect_frame_buffer(struct razer_chroma_device *device)
{
	struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
	if(device_data->frame_buffer_linked_uid != 0) //unlink old render node first
	{
		struct razer_fx_render_node *old_rn = daemon_get_render_node(device_data->daemon,device_data->frame_buffer_linked_uid);
		old_rn->output_frame = razer_create_rgb_frame(device->columns_num,device->rows_num);
		old_rn->output_frame_linked_uid = -1;
	}
	//if(render_node->output_frame_linked_uid == -1)
	//	razer_free_rgb_frame(render_node->output_frame);
	//daemon->frame_buffer = razer_create_rgb_frame();
	device_data->frame_buffer_linked_uid = -1;
	device_data->is_render_nodes_dirty = 1;
}

void daemon_connect_input(struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node)
{
	struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)render_node->device->tag;
	//if(render_node->input_frame_linked_uid != 0) //unlink old render node first
	//{
	//	struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
	//	old_rn->output_frame = daemon_create_rgb_frame();
	//	old_rn->output_frame_linked_uid = -1;
	//}
	if(render_node->input_frame_linked_uid == -1)
		razer_free_rgb_frame(render_node->input_frame);
	render_node->input_frame = input_node->output_frame;
	render_node->input_frame_linked_uid = input_node->id;
	device_data->is_render_nodes_dirty = 1;
}

void daemon_connect_second_input(struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node)
{
	struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)render_node->device->tag;
	//if(render_node->input_frame_linked_uid != 0) //unlink old render node first
	//{
	//	struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
	//	old_rn->output_frame = daemon_create_rgb_frame();
	//	old_rn->output_frame_linked_uid = -1;
	//}
	if(render_node->second_input_frame_linked_uid == -1)
		razer_free_rgb_frame(render_node->second_input_frame);
	render_node->second_input_frame = input_node->output_frame;
	render_node->second_input_frame_linked_uid = input_node->id;
	device_data->is_render_nodes_dirty = 1;
}

void daemon_set_default_render_node(struct razer_chroma_device *device,struct razer_fx_render_node *render_node)
{
	//setting the default daemon render node to render_node
	struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
	device_data->default_render_node = render_node;
}

int daemon_has_render_node_reached_render_limit(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(render_node->limit_render_time_ms)
	{
		//printf("checking render_limit %d : %u > %u + %u (%u)\n",render_node->id,daemon->chroma->update_ms,render_node->start_ticks,render_node->limit_render_time_ms,render_node->start_ticks + render_node->limit_render_time_ms);
		if(daemon->chroma->update_ms > (render_node->start_ticks + render_node->limit_render_time_ms))
		{
			#ifdef USE_DEBUGGING
				printf("\nreached render limit for node: %d\n",render_node->id);
			#endif
			return(1);
		}
	}
	return(0);
}

int daemon_update_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(!render_node || !render_node->effect)
		return(-1);

	if(!render_node->start_ticks)
	{
		render_node->start_ticks = razer_get_ticks();
		render_node->running  = 1;
		for(int i=0;i<list_GetLen(render_node->subs);i++)
		{
			struct razer_fx_render_node *sub = list_Get(render_node->subs,i);
			sub->start_ticks = 0;
			sub->running  = 1;
		}
	}
	if(!render_node->running || daemon_has_render_node_reached_render_limit(daemon,render_node))
		return(0);
	if(list_GetLen(render_node->subs))
	{
		/*#ifdef USE_DEBUGGING
			printf("## has compute nodes ##");
		#endif*/
		for(int i=0;i<list_GetLen(render_node->subs);i++)
		{
			struct razer_fx_render_node *sub = list_Get(render_node->subs,i);
			if(!sub->effect)
				continue;
			if(sub->effect->effect_class&=2)
				continue;//only execute compute effects
			if(!sub->start_ticks)
			{
				sub->start_ticks = razer_get_ticks();
				sub->running  = 1;
			}
			if(!sub->running)
				continue;
			int sub_ret = daemon_update_render_node(daemon,sub);
			if(!sub_ret || daemon_has_render_node_reached_render_limit(daemon,sub) || !sub->running)
			{
				if(sub->next)
				{
					list_Set(render_node->subs,i,sub->next);
					sub->next->parent = render_node;
					sub->next->start_ticks = 0; 
				}
				sub->running = 0;
				//return(0);
			}
		}
	}
	if(!render_node->effect->update)
		return(-1);
	int ret = render_node->effect->update(render_node);
	return(ret);
}

int daemon_handle_event_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,struct razer_chroma_event *event)
{
	if(!render_node || !render_node->effect)
		return(-1);

	if(!render_node->start_ticks)
	{
		render_node->start_ticks = razer_get_ticks();
		render_node->running  = 1;
	}
	if(!render_node->running || daemon_has_render_node_reached_render_limit(daemon,render_node))
		return(0);
	if(list_GetLen(render_node->subs))
	{
		for(int i=0;i<list_GetLen(render_node->subs);i++)
		{
			struct razer_fx_render_node *sub = list_Get(render_node->subs,i);
			if(!sub->start_ticks)
			{
				sub->start_ticks = razer_get_ticks();
				sub->running  = 1;
			}
			if(!sub->running)
				continue;
			int sub_ret = daemon_handle_event_render_node(daemon,sub,event);
			if(!sub_ret || daemon_has_render_node_reached_render_limit(daemon,sub) || !sub->running)
			{
				if(sub->next)
				{
					list_Set(render_node->subs,i,sub->next);
					sub->next->parent = render_node;
					sub->next->start_ticks = 0; 
				}
				sub->running = 0;
				//return(0);
			}
		}
	}
	if(!render_node->effect->handle_event)
		return(-1);
	int ret = render_node->effect->handle_event(render_node,event);
	return(ret);
}
