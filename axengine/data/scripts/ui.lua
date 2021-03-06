--[[
   ui.lua
--]]
WindowEvent = {};
WindowEvent.onClose = "onWindowClose";--自定义消息
WindowEvent.closeButUp = function(self,evt)
  local prt = evt.target.parent;
  prt:dispatch(self.onClose,"");
  prt:close();
end
WindowEvent.setTop = function(self,evt)
  evt.target:setTop();
end

WindowEvent.drag_start = function(self,evt)
	evt.target:startDrag({width=-1,height=-1});
	evt.target:addListener(MouseEvent.SuperUp,"WindowEvent.drag_end","");
end

WindowEvent.drag_end = function(self,evt)
	evt.target:stopDrag();
	evt.target:removeListener(MouseEvent.SuperUp,"WindowEvent.drag_end","");
end

ButtonEvent = {};
ButtonEvent.onMouseUp = function(self,evt)
	evt.target:stopDrag();
	evt.target:removeListener(MouseEvent.SuperUp,"ButtonEvent.onMouseUp","");
end

ButtonEvent.onMouseDown = function(self,evt)
	local yy = evt.link.extend.y - 17 - 17 - evt.target.extend.y;
	evt.target:startDrag({x=0,y=17,width=0,height=yy});
	evt.target:addListener(MouseEvent.SuperUp,"ButtonEvent.onMouseUp","");
end

ButtonEvent.onMouseIn = function(self,evt)
	 evt.target.keyFrame = 1;
end

ButtonEvent.onMouseOut = function(self,evt)
	 evt.target.keyFrame = 0; 
end

function tree_init(evt)
	local root = evt.target:getRootItem();
	root:insert("我");
	root:insert("grassItem2");
	local item1 = root:insert("treeItem0");
	item1:insert("treeItem6");
	item1:insert("treeItem7");
	local item2 = item1:insert("treeItem3");
	item2:insert("treeItem4");
	item2:insert("tree");
	item2:insert("grass");
	root:insert("treeItem5");
	item1 = root:insert("treeItem4");
	item1:insert("treeItem6");
	item1:insert("treeItem7");
	evt.target:updateTree();
end
function list_init(evt)
	for i=1,15 do
		evt.target:addItem("item"..tostring(i));
	end;
	evt.target:onResize(evt);
end
  
function myWinCloseFun(evt)
  print("winClose!");
end 