package ee.ttu.deko.coffee.service.domain;

/**
 * Coffee machine product state
 */
public enum ProductState {
    READY_TO_START,
    STARTING,
    IN_PROGRESS,
    FAILED,
    STOPPED
}
