BUILD_DIR = build
EXECUTABLE = GuitarVisualizer

.PHONY: build run clean rebuild

build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DOF_ROOT=/opt/openFrameworks .. && make -j$$(nproc)

run: build
	@./$(BUILD_DIR)/$(EXECUTABLE)

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Diretório de build está limpo"

rebuild: clean build