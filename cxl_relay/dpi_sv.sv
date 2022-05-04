/*************************
 * Filename : dpi_test.sv
 *************************/
`timescale 1 ps/ 1 ps
`define MAX_PAYLOAD_SIZE 256
typedef struct 
               {  
                  int unsigned packet_number;
                  int unsigned packet_type;
                  int unsigned sim_type; 
                  longint unsigned base_address;
                  longint unsigned physical_address;
                  int     unsigned r0w1;
                  int     unsigned data_size;
                  byte    unsigned data[`MAX_PAYLOAD_SIZE];
               } simics_transaction_t;

module dpi_sv();

   import "DPI-C" function int open_server_fifo();
   import "DPI-C" function void print_simics_pkt_data( input simics_transaction_t pkt);
   import "DPI-C" function int simics_dpi_request( output simics_transaction_t from_simics);
   import "DPI-C" function int simics_dpi_response( input simics_transaction_t to_simics);


   initial 
     begin
        $display("dpi_test.sv:  SV SIDE Initializing the shared lib");
        open_server_fifo();
        run();
     end

   task run();
      begin
         int                  ret;
         simics_transaction_t req;
         simics_transaction_t rsp;
         
         forever
	   begin
	      ret=simics_dpi_request(req);
	      if(ret<0) $finish;
              
	      $display("%m: Received a SIMICS packet");
              print_simics_pkt_data(req);

              if (req.packet_number > 0) begin
                 // copy req into rsp
                 rsp.packet_number    = req.packet_number;
                 rsp.packet_type      = req.packet_type;
                 rsp.sim_type         = req.sim_type;
                 rsp.base_address     = req.base_address;
                 rsp.physical_address = req.physical_address;
                 rsp.r0w1             = req.r0w1;
                 rsp.data_size        = req.data_size;
                 for(int i=0;i<req.data_size;i++)
                   begin
                      if (req.r0w1 == 0)
                        rsp.data[i] = req.packet_number + i;
                      else
                        rsp.data[i] = req.data[i];
                   end
                 
	         ret=simics_dpi_response(rsp);
                 print_simics_pkt_data(rsp);              
                 $display("%m:  SV SIDE replying back DONE, Exit...");
              end else begin // if (req.packet_number > 0)
                 $finish("Received packet with packet_number 0, THE TEST IS FINISHED!!");
              end // else: !if(req.packet_num > 0)
              

           end // forever begin
      end
      
   endtask // run
   

endmodule // dpi_sv




/* -----\/----- EXCLUDED -----\/-----
 $write("%m: simics_dpi_read_request: packet_number=%10d, packet_type=%2d, sim_type=%2d, base_address=%016X, physical_address=%016X, r0w1=%1d, data_size=%4d\n", packet_number, packet_type, sim_type, base_address, physical_address, r0w1, data_size);
 $write("data:");
 for(i=0;i<data_size;i++)
 begin
 $write("%02X ",cxl_data[i]);
 cxl_data[i] = ~cxl_data[i]; // invert data just to be different
                end
 $write("\n");									
 $display("%m:  SV SIDE replying back to host");
 -----/\----- EXCLUDED -----/\----- */

